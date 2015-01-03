
#include <fstream>

#include <QAction>
#include <QTimer>
#include <QtWidgets>
#include <QFileDialog>

#include "novelviewer.hpp"

class QMenuButton : public QWidgetAction
{
	QAbstractButton* m_button;
public:
    explicit QMenuButton(QAbstractButton* button, QObject* parent)
		: QWidgetAction(parent)
	{
		(m_button = button);

		setDefaultWidget(m_button);
	}

    virtual ~QMenuButton()
	{
//		if (m_button)
//			m_button->deleteLater();
	}

    virtual QWidget* createWidget(QWidget* parent)
	{
		parent->layout()->addWidget(m_button);
//		m_button->setParent(parent);
		auto r = m_button;
		m_button = NULL;
		return r;
	}

};

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya_download(io, info, true)
{
	ui.setupUi(this);

	m_toolbar = new QToolBar(this);

	addToolBar(Qt::TopToolBarArea, m_toolbar);

	m_title = info.title;

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	ui.textBrowser->clear();

	connect(&m_tianya_download, SIGNAL(chunk_download_notify(QString)), ui.textBrowser, SLOT(append(QString)));


	connect(&m_tianya_download, &tianya_download::chunk_download_notify, this, [this](QString)
	{
		statusBar()->showMessage(QStringLiteral("下载中..."));
	});


 	connect(&m_tianya_download, SIGNAL(timed_first_timershot()), this, SLOT(text_brower_stay_on_top()));

	connect(&m_tianya_download, SIGNAL(download_complete()), this, SLOT(download_complete()));

	m_tianya_download.start_download();

	m_action_send_to_kindle = m_toolbar->addAction(QStringLiteral("发送到 kindle(&K)"));
	m_action_send_to_kindle->setShortcuts({QKeySequence("Ctrl+K"), QKeySequence("Alt+K")});
	m_toolbar->addSeparator();

	m_action_save_to_file = m_toolbar->addAction(QStringLiteral("保存文件(&S)"));
	m_action_save_to_file->setShortcuts({QKeySequence("Ctrl+S"), QKeySequence("Alt+S")});

	m_toolbar->addSeparator();

	auto cbox = new QCheckBox(QStringLiteral("过滤楼主回复贴(&R)"));
	//auto cbox = m_toolbar->addAction(QStringLiteral("过滤楼主回复贴(&R)"));

	cbox->setCheckable(true);
	cbox->setChecked(m_tianya_download.filter_reply());

	connect(cbox, SIGNAL(clicked(bool)), &m_tianya_download, SLOT(set_filter_reply(bool)));

	m_toolbar->addWidget(cbox);

	connect(m_action_send_to_kindle, &QAction::hovered, this, [this]()
	{
		statusBar()->showMessage(QStringLiteral("将当前文章发送到 Kindle 设备"), 1);
	});

	connect(m_action_send_to_kindle, SIGNAL(triggered(bool)), this, SLOT(mail_to()));

	connect(m_action_save_to_file, &QAction::hovered, this, [this]()
	{
		statusBar()->showMessage(QStringLiteral("将当前文章保存到文件"), 1);
	});

	connect(m_action_save_to_file, SIGNAL(triggered(bool)), this, SLOT(save_to()));

	statusBar()->showMessage(QStringLiteral("下载中..."));

	m_progress_bar = new QProgressBar(this);

	statusBar()->addPermanentWidget(m_progress_bar);
	m_progress_bar->setMaximum(100);
	m_progress_bar->setMinimum(0);

	connect(&m_tianya_download, &tianya_download::download_progress_report, m_progress_bar, [this](double v){
		m_progress_bar->setValue(v*100);
	});

	connect(&m_tianya_download, &tianya_download::download_complete, m_progress_bar, [this]()
	{
		m_progress_bar->setValue(100);
		QTimer::singleShot(3000, m_progress_bar, SLOT(deleteLater()));
	});
}

NovelViewer::~NovelViewer()
{
}

bool NovelViewer::filter_reply()
{
	return m_tianya_download.property("filter_reply").toBool();
}

void NovelViewer::set_filter_reply(bool v)
{
	m_tianya_download.setProperty("filter_reply", v);
}

void NovelViewer::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type())
	{
	case QEvent::LanguageChange:
		ui.retranslateUi(this);
		break;
	default:
		break;
	}
}

void NovelViewer::text_brower_stay_on_top()
{
	ui.textBrowser->verticalScrollBar()->setValue(0);
}

void NovelViewer::download_complete()
{
	statusBar()->showMessage(QStringLiteral("下载完成"));
}

void NovelViewer::save_to()
{
	QFileDialog filedlg(this);

	filedlg.setAcceptMode(QFileDialog::AcceptSave);
	filedlg.setDefaultSuffix("txt");
	filedlg.setNameFilter(QStringLiteral("文本文件 (*.txt)"));

	QString suggested_filename = QString("%1.txt").arg(QString::fromStdWString(m_title));

	filedlg.selectFile(suggested_filename);

	if (filedlg.exec() == QFileDialog::Accepted && filedlg.selectedFiles().size())
	{
		QString savefilename = filedlg.selectedFiles().first();

		save_to_file(savefilename);
	}
}

void NovelViewer::save_to_file(QString filename)
{
	m_tianya_download.save_to_file(filename);
}

void NovelViewer::mail_to()
{
	QSettings settings;
	EmailAddress mail_rcpt(settings.value("kindle.kindlemail").toString().toUtf8().toStdString());

	auto progress_bar = new QProgressBar(this);

	statusBar()->addPermanentWidget(progress_bar);
	progress_bar->setFormat(QStringLiteral("发送邮件... %p%"));

	QObject::connect(&m_tianya_download, &tianya_download::mailsend_progress_report, progress_bar, [progress_bar, this](double v)
	{
		progress_bar->setValue(v * 100);
	});

	QObject::connect(&m_tianya_download, &tianya_download::send_complete, progress_bar, [progress_bar, this]()
	{
		progress_bar->setValue(100);

		progress_bar->setFormat(QStringLiteral("发送完成"));

		QTimer::singleShot(3000, progress_bar, SLOT(deleteLater()));
	});

	m_tianya_download.start_send_mail(mail_rcpt);
}

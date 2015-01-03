
#include <fstream>

#include <QTimer>
#include <QtWidgets>
#include <QFileDialog>

#include "novelviewer.hpp"

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya_download(io, info)
{
	ui.setupUi(this);

	m_title = info.title;

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	ui.textBrowser->clear();

	connect(&m_tianya_download, SIGNAL(chunk_download_notify(QString)), ui.textBrowser, SLOT(append(QString)));
 	connect(&m_tianya_download, SIGNAL(timed_first_timershot()), this, SLOT(text_brower_stay_on_top()));

	connect(&m_tianya_download, SIGNAL(download_complete()), this, SLOT(download_complete()));

	m_tianya_download.start();

	m_action_send_to_kindkle = menuBar()->addAction(QStringLiteral("发送到 kindle(&K)"));
	m_action_save_to_file = menuBar()->addAction(QStringLiteral("保存文件(&S)"));

	m_action_save_to_file->setShortcut(QKeySequence("Ctrl+S"));


	connect(m_action_send_to_kindkle, &QAction::hovered, this, [this]()
	{
		statusBar()->showMessage(QStringLiteral("将当前文章发送到 Kindle 设备"));
	});

	connect(m_action_save_to_file, &QAction::hovered, this, [this]()
	{
		statusBar()->showMessage(QStringLiteral("将当前文章保存到文件"));
	});

	connect(m_action_save_to_file, SIGNAL(triggered(bool)), this, SLOT(save_to()));

	statusBar()->showMessage(QStringLiteral("下载中..."));
}

NovelViewer::~NovelViewer()
{
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

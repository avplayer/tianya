#include <QTimer>
#include <QScrollBar>
#include "novelviewer.hpp"

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya_download(io, info)
{
	ui.setupUi(this);

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	ui.textBrowser->clear();

	connect(&m_tianya_download, SIGNAL(chunk_download_notify(QString)), ui.textBrowser, SLOT(append(QString)));
 	connect(&m_tianya_download, SIGNAL(timed_first_timershot()), this, SLOT(text_brower_stay_on_top()));

	connect(&m_tianya_download, SIGNAL(download_complete()), this, SLOT(download_complete()));

	m_tianya_download.start();

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


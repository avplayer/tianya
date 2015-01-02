#include <QTimer>
#include <QScrollBar>
#include "novelviewer.hpp"

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_first_append(true)
	, m_tianya_download(io, info)
	, m_quited(std::make_shared<bool>(false))
{
	ui.setupUi(this);

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	ui.textBrowser->clear();

	auto quited = m_quited;

	connect(&m_tianya_download, SIGNAL(chunk_download_notify(QString)), ui.textBrowser, SLOT(append(QString)));
 	connect(&m_tianya_download, SIGNAL(timed_first_timershot()), this, SLOT(text_brower_stay_on_top()));

	m_tianya_download.start();
}

NovelViewer::~NovelViewer()
{
	* m_quited = true;
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


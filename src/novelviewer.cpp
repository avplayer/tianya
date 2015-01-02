#include "syncobj.hpp"
#include "novelviewer.hpp"
#include <tianya_context.hpp>

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_list_info(info)
{
	ui.setupUi(this);

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	m_tianya_context.reset(new tianya_context(m_io_service));

	m_tianya_context->connect_stoped([this]()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		m_download_complete = true;
		m_tianya_stopped.notify_all();
	});

	m_tianya_context->connect_one_content_fetched([this](std::wstring content)
	{
		post_on_gui_thread([this, content]()
		{
			// 向文本控件添加文字.

			ui.textBrowser->append(QString::fromStdWString(content));

		});
	});

	m_tianya_context->start(m_list_info.post_url);
}

NovelViewer::~NovelViewer()
{
	std::unique_lock<std::mutex> l(m_mutex);

	m_tianya_context->stop();
	// wait until real stop
	if (!m_download_complete)
		m_tianya_stopped.wait(l);
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

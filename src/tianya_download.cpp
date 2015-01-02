
#include <boost/asio.hpp>
#include <QTimer>

#include "syncobj.hpp"
#include "tianya_download.hpp"

tianya_download::tianya_download(boost::asio::io_service& io, const list_info& info, QObject* parent)
	: QObject(parent)
	, m_io_service(io)
	, m_tianya_context(io)
	, m_list_info(info)
	, m_should_quit(std::make_shared<bool>(false))
	, m_first_chunk(true)
{
	auto should_quit = m_should_quit;
	m_tianya_context.connect_one_content_fetched([this, should_quit](std::wstring content)
	{
		post_on_gui_thread([this, should_quit, content]()
		{
			if (*should_quit)
				return;
			chunk_download_notify(QString::fromStdWString(content));

			if (m_first_chunk)
			{
				m_first_chunk = false;

				QTimer::singleShot(200, this, [this](){timed_first_timershot();});
			}
		});
	});
}

tianya_download::~tianya_download()
{
	*m_should_quit = true;
	m_tianya_context.stop();
}

void tianya_download::start()
{
	m_tianya_context.start(m_list_info.post_url);
}

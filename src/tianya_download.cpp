
#include <memory>
#include <functional>

#include <boost/asio.hpp>
#include <QTimer>
#include <QtWidgets>
#include <QFile>

#include "mimemessage.hpp"

#include "syncobj.hpp"
#include "tianya_download.hpp"

tianya_download::tianya_download(boost::asio::io_service& io, const list_info& info, bool filter_reply, QObject* parent)
	: QObject(parent)
	, m_io_service(io)
	, m_tianya_context(std::make_shared<tianya_context>(std::ref(io)))
	, m_list_info(info)
	, m_first_chunk(true)
	, m_is_gone(std::make_shared<bool>(false))
{
	auto is_gone = m_is_gone;
	m_connection_notify_chunk = m_tianya_context->connect_one_content_fetched([this, is_gone](std::wstring content)
	{
		double progressprecent = (double)m_tianya_context->page_index() / (double) m_tianya_context->page_count();
		post_on_gui_thread([this, is_gone, content, progressprecent]()
		{
			if (*is_gone)
				return;
			chunk_download_notify(QString::fromStdWString(content));

			download_progress_report(progressprecent);

			if (m_first_chunk)
			{
				m_first_chunk = false;

				QTimer::singleShot(200, this, [this](){timed_first_timershot();});
			}
		});
	});

	m_connection_notify_complete = m_tianya_context->connect_download_complete([this, is_gone]()
	{
		post_on_gui_thread([this, is_gone]()
		{
			if (*is_gone)
				return;
			download_complete();
		});
	});

	m_tianya_context->filter_reply(filter_reply);
}

bool tianya_download::filter_reply()
{
	return m_tianya_context->filter_reply();
}

void tianya_download::set_filter_reply(bool v)
{
	m_tianya_context->filter_reply(v);
}

tianya_download::~tianya_download()
{
	*m_is_gone = true;
	m_connection_notify_complete.disconnect();
	m_connection_notify_chunk.disconnect();

	m_tianya_context->stop();
}

void tianya_download::start_download()
{
	m_tianya_context->start(m_list_info.post_url);
}

void tianya_download::save_to_file(QString filename)
{
	std::shared_ptr<QFile> filestream = std::make_shared<QFile>(filename);

	filestream->open(QFile::WriteOnly);

	if (filestream->isOpen())
	{
		// add BOM
		filestream->write("\357\273\277", 3);

		auto _tianya_context = m_tianya_context;

		m_io_service.post([filestream, _tianya_context]()
		{
			_tianya_context->serialize_to_io_device(filestream.get());
		});
	}
}

void tianya_download::start_send_mail(EmailAddress target)
{
	


}


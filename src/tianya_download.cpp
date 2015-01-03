
#include <memory>
#include <functional>

#include <boost/asio.hpp>
#include <QTimer>
#include <QtWidgets>
#include <QFile>

#include "mimemessage.hpp"
#include "mimeattachment.hpp"
#include "mimetext.hpp"

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

void tianya_download::start_send_mail(EmailAddress mail_rcpt)
{
	QSettings settings;

	EmailAddress mail_sender(settings.value("kindle.usermail").toString().toUtf8().toStdString() , "Tianya Radar");
	QString password = settings.value("kindle.usermail_passwd").toString();

	m_smtp.reset(new mx::smtp(m_io_service, mail_sender.getAddress(), password.toUtf8().toStdString()));

	InternetMailFormat imf;

	imf.header["from"] = mail_sender.getAddress();
	imf.header["to"] = mail_rcpt.getAddress();
	imf.header["subject"] = "Convert";

	std::shared_ptr<MimeMessage> message = std::make_shared<MimeMessage>();

	message->setSender(mail_sender);
	message->addRecipient(mail_rcpt);
    message->setSubject("Convert");

	auto text_part = std::make_shared<MimeText>();
    text_part->setText("please convert this attachment and send it to my kindle");
	message->addPart(text_part.get());

	QBuffer articlecontent;
	articlecontent.open(QBuffer::ReadWrite);

	m_tianya_context->serialize_to_io_device(&articlecontent);

	articlecontent.seek(0);

	std::shared_ptr<MimeAttachment> attachment;

	attachment.reset(new MimeAttachment(articlecontent.buffer(), QStringLiteral("%1.txt").arg(QString::fromStdWString(m_list_info.title))));

	message->addPart(attachment.get());

	mailsend_progress_report(0.5);

	imf.custom_data = [message, text_part, attachment](std::ostream* o)
	{
		*o << message->toString().toUtf8().toStdString();
	};

	auto is_gone = m_is_gone;
	m_smtp->async_sendmail(imf, [this, is_gone](const boost::system::error_code & ec)
	{
		if (*is_gone)
			return;

		post_on_gui_thread([this, is_gone]()
		{
			if (*is_gone)
				return;
			// nice
			mailsend_progress_report(1.0);

			send_complete();
		});
	});
}


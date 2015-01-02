#pragma once

#include <QObject>
#include <boost/asio/io_service.hpp>
#include <tianya_context.hpp>
#include <tianya_list.hpp>

class tianya_download : public QObject
{
	Q_OBJECT

public:
	explicit tianya_download(boost::asio::io_service&, const list_info&, QObject* parent = 0);

	virtual ~tianya_download();

	std::shared_ptr<tianya_context> get_tianya_context(){return m_tianya_context;}

public Q_SLOTS:
	void start();

Q_SIGNALS:
	void download_complete();
	void chunk_download_notify(QString);
	void timed_first_timershot();

private:
	boost::asio::io_service& m_io_service;
	list_info m_list_info;

	boost::signals2::scoped_connection m_connection_notify_chunk;
	boost::signals2::scoped_connection m_connection_notify_complete;

	std::shared_ptr<tianya_context> m_tianya_context;

	bool m_first_chunk;
};


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

public Q_SLOTS:
	void start();

Q_SIGNALS:
	void download_complete();
	void chunk_download_notify(QString);
	void timed_first_timershot();

private:
	boost::asio::io_service& m_io_service;
	tianya_context m_tianya_context;
	list_info m_list_info;


	std::shared_ptr<bool> m_should_quit;
	bool m_first_chunk;
};


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

Q_SIGNALS:


	void download_complete();
	void chunk_download_notify(std::wstring);

private:
	boost::asio::io_service& m_io_service;
	tianya_context m_tianya_context;
};


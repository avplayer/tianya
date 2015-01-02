#pragma once

#include <condition_variable>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <tianya_list.hpp>

#include "ui_novelviewer.h"

class tianya_context;
class NovelViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit NovelViewer(boost::asio::io_service&, list_info, QWidget *parent = 0);
	~NovelViewer();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::NovelViewer ui;
	boost::asio::io_service& m_io_service;

	list_info m_list_info;

	std::shared_ptr<tianya_context> m_tianya_context;

	std::atomic<bool> m_download_complete;
//	std::atomic<bool> m_delete_later;
	mutable std::mutex m_mutex;
	std::condition_variable m_tianya_stopped;
};

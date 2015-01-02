#pragma once

#include <condition_variable>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <tianya_context.hpp>
#include <tianya_list.hpp>
#include "tianya_download.hpp"

#include "ui_novelviewer.h"

class NovelViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit NovelViewer(boost::asio::io_service&, list_info, QWidget *parent = 0);
	virtual ~NovelViewer();

protected:
	void changeEvent(QEvent *e);

private Q_SLOTS:
	void text_brower_stay_on_top();
	void download_complete();

private:
	Ui::NovelViewer ui;
	boost::asio::io_service& m_io_service;
	tianya_download m_tianya_download;
};

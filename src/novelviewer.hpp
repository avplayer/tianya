#pragma once

#include <condition_variable>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <tianya_context.hpp>
#include <tianya_list.hpp>

#include "ui_novelviewer.h"

class NovelViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit NovelViewer(boost::asio::io_service&, list_info, QWidget *parent = 0);
	~NovelViewer();

protected:
	void changeEvent(QEvent *e);

private Q_SLOTS:
	void text_brower_stay_on_top();

private:
	Ui::NovelViewer ui;
	boost::asio::io_service& m_io_service;

	list_info m_list_info;

	tianya_context m_tianya_context;

	bool m_first_append;
};

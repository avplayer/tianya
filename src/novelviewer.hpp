
#pragma once

#include <condition_variable>
#include <mutex>
#include <boost/asio/io_service.hpp>

#include <QWidget>
#include <QMainWindow>
#include <QToolBar>

#include <tianya_context.hpp>
#include <tianya_list.hpp>
#include "tianya_download.hpp"


#include "ui_novelviewer.h"

class NovelViewer : public QMainWindow
{
	Q_OBJECT

	Q_PROPERTY(bool filter_reply READ filter_reply WRITE set_filter_reply);

public:
	explicit NovelViewer(boost::asio::io_service&, list_info, QWidget *parent = 0);
	virtual ~NovelViewer();

	bool filter_reply();

protected:
	void changeEvent(QEvent *e);

private Q_SLOTS:
	void text_brower_stay_on_top();
	void download_complete();

	void save_to_file(QString);

	void save_to();
	void set_filter_reply(bool);


private:
	Ui::NovelViewer ui;
	boost::asio::io_service& m_io_service;
	std::wstring m_title;
	tianya_download m_tianya_download;

	QAction* m_action_send_to_kindkle;
    QAction* m_action_save_to_file;
    QToolBar* m_toolbar;
};

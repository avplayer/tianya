#pragma once

#include <string>
#include <boost/asio/io_service.hpp>
#include <tianya_list.hpp>
#include "ui_tianyawindow.h"

class TianyaWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TianyaWindow(boost::asio::io_service& , QWidget* parent = 0);

    void show();

protected:
	void changeEvent(QEvent *e);


private:
	Ui::TianyaWindow ui;
	boost::asio::io_service& m_io_service;

	tianya m_tianya;

	std::string m_post_url;
};

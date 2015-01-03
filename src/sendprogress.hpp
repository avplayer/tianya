
#pragma once

#include <QtWidgets>
#include <tianya_list.hpp>

#include "tianya_download.hpp"
#include "ui_sendprogress.h"

class SendProgress : public QGroupBox
{
    Q_OBJECT
public:
    explicit SendProgress(boost::asio::io_service&, const list_info&, QWidget* parent = 0);

public Q_SLOTS:

	// 开始.
	void start();

	void set_work_percent(double);

private Q_SLOTS:

	void start_sendmail();


private:
    Ui::SendProgress ui;

	tianya_download m_tianya_download;
};

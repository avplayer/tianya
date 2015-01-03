
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


private Q_SLOTS:

	void start_sendmail();

	void set_download_work_percent(double);
	void set_send_work_percent(double);

	void mail_sended();

private:
    Ui::SendProgress ui;

	tianya_download m_tianya_download;
};

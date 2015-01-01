#pragma once

#include <string>
#include <boost/asio/io_service.hpp>
#include <tianya_list.hpp>
#include <QSortFilterProxyModel>
#include "tianyamodel.hpp"
#include "ui_tianyawindow.h"


class TianyaWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TianyaWindow(boost::asio::io_service& , QWidget* parent = 0);
	~TianyaWindow();

protected:
	virtual void changeEvent(QEvent *e) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:
	void on_tableView_doubleClicked(const QModelIndex &index);
	void timer_adjust_Column();
	void real_start_tianya();

private:
	Ui::TianyaWindow ui;
	boost::asio::io_service& m_io_service;

	bool m_fist_insertion;

	tianya m_tianya;

	TianyaModel m_tianya_data_mode;
	QSortFilterProxyModel m_sortproxy_for_tianya_data_mode;

	std::string m_post_url;
};

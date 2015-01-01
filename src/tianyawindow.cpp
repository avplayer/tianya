
#include <QDesktopServices>
#include <QUrl>

#include "tianyawindow.hpp"
#include "syncobj.hpp"

TianyaWindow::TianyaWindow(boost::asio::io_service& io, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya(m_io_service)
{
	ui.setupUi(this);

	m_post_url = "http://bbs.tianya.cn/list.jsp?item=culture&grade=1&order=1";

	m_sortproxy_for_tianya_data_mode.setSourceModel(&m_tianya_data_mode);

	// 设置 modle !
	ui.tableView->setModel(&m_sortproxy_for_tianya_data_mode);
	ui.tableView->setAlternatingRowColors(true);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSortingEnabled(true);
	ui.tableView->sortByColumn(2, Qt::DescendingOrder);
}

void TianyaWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type())
	{
	case QEvent::LanguageChange:
		ui.retranslateUi(this);
		break;
	default:
		break;
	}
}

void TianyaWindow::show()
{
	QMainWindow::show();

	m_tianya.connect_hit_item_fetched([this](list_info hits_info)
	{
		post_on_gui_thread([this, hits_info]()
		{
			m_tianya_data_mode.update_tianya_list(hits_info);
		});
	});

	m_tianya.start(m_post_url);
}

TianyaWindow::~TianyaWindow()
{
	m_tianya.stop();
}

void TianyaWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
	QDesktopServices::openUrl(index.data(Qt::UserRole+1).toUrl());
}

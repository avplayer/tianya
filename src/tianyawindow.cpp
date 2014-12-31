#include "tianyawindow.hpp"

TianyaWindow::TianyaWindow(boost::asio::io_service& io, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya(m_io_service)
{
	ui.setupUi(this);

	m_post_url = "http://bbs.tianya.cn/list.jsp?item=culture&grade=1&order=1";

	// 设置 modle !
	ui.tableView->setModel(&m_tianya_data_mode);
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

	m_tianya.connect_hits_changed([this](const std::multimap<int, list_info>& ordered_info)
	{
		m_tianya_data_mode.update_tianya_list(ordered_info);
	});

	m_tianya.start(m_post_url);
}

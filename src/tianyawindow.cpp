#include "tianyawindow.hpp"

TianyaWindow::TianyaWindow(boost::asio::io_service& io, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya(m_io_service)
	, m_tianya_data_mode(m_tianya)
{
	ui.setupUi(this);

	m_post_url = "http://bbs.tianya.cn/list.jsp?item=culture&grade=1&order=1";
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

	m_tianya.start(m_post_url);

	// 设置 modle !
	ui.tableView->setModel(&m_tianya_data_mode);
}

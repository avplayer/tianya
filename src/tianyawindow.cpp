
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTableView>
#include <QTimer>
#include <QUrl>
#include <QWidgetAction>

#include "tianyawindow.hpp"
#include "syncobj.hpp"
#include "kindlesettingdialog.hpp"

TianyaWindow::TianyaWindow(boost::asio::io_service& io, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_tianya(m_io_service)
	, m_fist_insertion(true)
{
	ui.setupUi(this);

	m_post_url = "http://bbs.tianya.cn/list.jsp?item=culture&grade=1&order=1";

	m_sortproxy_for_tianya_data_mode.setSourceModel(&m_tianya_data_mode);
	m_sortproxy_for_tianya_data_mode.setFilterKeyColumn(0);

	// 设置 modle !
	ui.tableView->setModel(&m_sortproxy_for_tianya_data_mode);
	ui.tableView->setAlternatingRowColors(true);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSortingEnabled(true);
	ui.tableView->sortByColumn(2, Qt::DescendingOrder);
	ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui.tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pop_up_context_menu(QPoint)));

	QTimer::singleShot(20, this, SLOT(real_start_tianya()));

	// 菜单
	setMenuWidget(new QWidget(this));

	auto hbox = new QHBoxLayout(menuWidget());

	auto qsetkindke_button = new QPushButton(QStringLiteral("设置Kindle邮箱"), menuWidget());
	hbox->addWidget(qsetkindke_button);

	hbox->addWidget(new QFrame(menuWidget()));

	auto qlineedit = new QLineEdit(menuWidget());
	hbox->addWidget(qlineedit);

	qlineedit->setPlaceholderText(QStringLiteral("输入文字过滤标题"));

	connect(qlineedit, SIGNAL(textChanged(QString)), &m_sortproxy_for_tianya_data_mode, SLOT(setFilterFixedString(QString)));

	connect(qsetkindke_button, SIGNAL(clicked(bool)), this, SLOT(kindle_settings(bool)));
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

void TianyaWindow::closeEvent(QCloseEvent*e)
{
    QWidget::closeEvent(e);

	m_tianya.stop();
}

void TianyaWindow::real_start_tianya()
{
	m_tianya.connect_hit_item_fetched([this](list_info hits_info)
	{
		post_on_gui_thread([this, hits_info]()
		{
			if (m_fist_insertion)
				QTimer::singleShot(300, this, SLOT(timer_adjust_Column()));
			m_fist_insertion = false;

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

void TianyaWindow::timer_adjust_Column()
{
	ui.tableView->resizeColumnsToContents();
}

void TianyaWindow::pop_up_context_menu(QPoint pos)
{
	auto pop_menu = new QMenu(this);

	auto action = pop_menu->addAction("发送到 Kindle");

	auto indexes = ui.tableView->selectionModel()->selectedIndexes();

	connect(action, &QAction::triggered, this, [indexes, this](bool)
	{
		// 这个用 lambda 是为了捕获上下文

		for (QModelIndex i : indexes)
		{
			QVariant post_url_var = i.data(Qt::UserRole+1);
			if (post_url_var.isValid())
			{
				QUrl post_url = post_url_var.toUrl();

				// 构造 tianyadownload 对象, 下载 TXT 然后以邮件附件形式发送到 kindle 里.
			}
		}

	});

	pop_menu->popup(ui.tableView->viewport()->mapToGlobal(pos));

	pop_menu->setAttribute(Qt::WA_DeleteOnClose);
}

void TianyaWindow::kindle_settings(bool)
{
	KindleSettingDialog settingsdialog(this);

	QSettings settings;

	settingsdialog.setProperty("kindlemail", settings.value("kindle.kindlemail"));
	settingsdialog.setProperty("usermail", settings.value("kindle.usermail"));
	settingsdialog.setProperty("usermail_passwd", settings.value("kindle.usermail_passwd"));

	if (settingsdialog.exec() == QDialog::Accepted)
	{
		// 更新设置.
		settings.setValue("kindle.kindlemail", settingsdialog.property("kindlemail"));
		settings.setValue("kindle.usermail", settingsdialog.property("usermail"));
		settings.setValue("kindle.usermail_passwd", settingsdialog.property("usermail_passwd"));
	}
}

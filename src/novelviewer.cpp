#include <QTimer>
#include <QScrollBar>
#include "syncobj.hpp"
#include "novelviewer.hpp"

NovelViewer::NovelViewer(boost::asio::io_service& io, list_info info, QWidget *parent)
	: QMainWindow(parent)
	, m_io_service(io)
	, m_list_info(info)
	, m_first_append(true)
	, m_tianya_context(io)
	, m_quited(std::make_shared<bool>(false))
{
	ui.setupUi(this);

	auto title = QString("%1 - %2").arg(QString::fromStdWString(info.title)).arg(QString::fromStdWString(info.author));

	setWindowTitle(title);

	ui.textBrowser->clear();

	auto quited = m_quited;

	m_tianya_context.connect_one_content_fetched([this, quited](std::wstring content)
	{
		if (*quited)
			return;
		post_on_gui_thread([this, content]()
		{
			// 向文本控件添加文字.
			ui.textBrowser->append(QString::fromStdWString(content));

			if (m_first_append)
			{
				m_first_append = false;

				QTimer::singleShot(100, this, SLOT(text_brower_stay_on_top()));
			}
		});
	});

	m_tianya_context.start(m_list_info.post_url);
}

NovelViewer::~NovelViewer()
{
	* m_quited = true;
}

void NovelViewer::changeEvent(QEvent *e)
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

void NovelViewer::text_brower_stay_on_top()
{
	ui.textBrowser->verticalScrollBar()->setValue(0);
}


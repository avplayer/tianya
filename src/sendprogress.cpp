
#include <QObject>
#include <QString>

#include "sendprogress.hpp"

SendProgress::SendProgress(boost::asio::io_service& io, const list_info& info, QWidget* parent)
	: QGroupBox(parent)
	, m_tianya_download(io, info, true)
{
	// 连接 signals 以更新 UI
	ui.setupUi(this);

	ui.progressBar->setFormat(QStringLiteral("%p% - (正在下载全文)"));
	ui.progressBar->setValue(0);

	QObject::connect(&m_tianya_download, SIGNAL(download_progress_report(double)), this, SLOT(set_work_percent(double)));
	QObject::connect(&m_tianya_download, SIGNAL(download_complete()), this, SLOT(start_sendmail()));

	ui.label_title->setText(QString::fromStdWString(info.title));
}

void SendProgress::start()
{
	m_tianya_download.start_download();
}

void SendProgress::set_work_percent(double v)
{
	ui.progressBar->setValue(v*80);
}

void SendProgress::start_sendmail()
{
	ui.progressBar->setValue(80);
	ui.progressBar->setFormat(QStringLiteral("%p% - (正在发送邮件)"));

	// TODO 开始邮件发送.
	//m_tianya_download.save_to_file();
}


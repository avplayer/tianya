
#include <QObject>
#include <QString>
#include <QSettings>

#include "sendprogress.hpp"

SendProgress::SendProgress(boost::asio::io_service& io, const list_info& info, QWidget* parent)
	: QGroupBox(parent)
	, m_tianya_download(io, info, true)
{
	// 连接 signals 以更新 UI
	ui.setupUi(this);

	ui.progressBar->setFormat(QStringLiteral("%p% - (正在下载全文)"));
	ui.progressBar->setValue(0);

	QObject::connect(&m_tianya_download, SIGNAL(download_progress_report(double)), this, SLOT(set_download_work_percent(double)));
	QObject::connect(&m_tianya_download, SIGNAL(download_complete()), this, SLOT(start_sendmail()));

	ui.label_title->setText(QString::fromStdWString(info.title));
}

void SendProgress::start()
{
	m_tianya_download.start_download();
}

void SendProgress::set_download_work_percent(double v)
{
	ui.progressBar->setValue(v*80);
}

void SendProgress::set_send_work_percent(double v)
{
	ui.progressBar->setValue(80 + v*20);
}

void SendProgress::start_sendmail()
{
	ui.progressBar->setValue(80);
	ui.progressBar->setFormat(QStringLiteral("%p% - (正在发送邮件)"));


	QSettings settings;

	EmailAddress mail_rcpt(settings.value("kindle.kindlemail").toString().toUtf8().toStdString());

	QObject::connect(&m_tianya_download, SIGNAL(mailsend_progress_report(double)), this, SLOT(set_send_work_percent(double)));
	QObject::connect(&m_tianya_download, SIGNAL(send_complete()), this, SLOT(mail_sended()));

	m_tianya_download.start_send_mail(mail_rcpt);
}

void SendProgress::mail_sended()
{
	QTimer::singleShot(2000, this, SLOT(deleteLater()));

	ui.progressBar->setValue(100);
	ui.progressBar->setFormat(QStringLiteral("%p% - (完成)"));
}



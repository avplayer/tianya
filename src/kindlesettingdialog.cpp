#include "kindlesettingdialog.hpp"

KindleSettingDialog::KindleSettingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

void KindleSettingDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type())
	{
	case QEvent::LanguageChange:
		ui.retranslateUi(this);
		break;
	default:
		break;
	}
}

QString KindleSettingDialog::get_kindlemail()
{
	return ui.kindle_mail->displayText();
}

void KindleSettingDialog::set_kindlemail(QString t)
{
	ui.kindle_mail->setText(t);
}

QString KindleSettingDialog::get_usermail()
{
	return ui.sendmail->displayText();
}

void KindleSettingDialog::set_usermail(QString t)
{
	ui.sendmail->setText(t);
}

QString KindleSettingDialog::get_usermail_passwd()
{
	return ui.sendmail_pwd->text();
}

void KindleSettingDialog::set_usermail_passwd(QString t)
{
	ui.sendmail_pwd->setText(t);
}

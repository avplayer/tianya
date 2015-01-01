#pragma once

#include "ui_kindlesettingdialog.h"

class KindleSettingDialog : public QDialog
{
	Q_OBJECT

	Q_PROPERTY(QString kindlemail READ get_kindlemail WRITE set_kindlemail);
	Q_PROPERTY(QString usermail READ get_usermail WRITE set_usermail);
	Q_PROPERTY(QString usermail_passwd READ get_usermail_passwd WRITE set_usermail_passwd);

public:
	explicit KindleSettingDialog(QWidget *parent = 0);

protected:
	void changeEvent(QEvent *e);

public:
	QString get_kindlemail();
	void set_kindlemail(QString);

	QString get_usermail();
	void set_usermail(QString);

	QString get_usermail_passwd();
	void set_usermail_passwd(QString);

private:
	Ui::KindleSettingDialog ui;
};

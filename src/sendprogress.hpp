
#pragma once

#include <QtWidgets>

#include "ui_sendprogress.h"

class SendProgress : public QMainWindow
{
    Q_OBJECT
public:
    explicit SendProgress(QWidget* parent = 0, Qt::WindowFlags flags = 0);


private:
    Ui::SendProgress ui;
};

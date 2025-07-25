#pragma once

#include <QDialog>
#include "ui_acceptcanceldialog.h"

class AcceptCancelDialog : public QDialog
{
	Q_OBJECT

public:
	AcceptCancelDialog(QWidget *parent, const std::string& message);
	~AcceptCancelDialog();
private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
private:
	Ui::AcceptCancelDialogClass ui;
};


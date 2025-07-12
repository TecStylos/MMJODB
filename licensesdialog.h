#pragma once

#include <QWidget>
#include "ui_licensesdialog.h"

class LicensesDialog : public QDialog
{
	Q_OBJECT

public:
	LicensesDialog(QWidget *parent);
	~LicensesDialog();

private:
	Ui::LicensesDialogClass ui;
};


#pragma once

#include <QDialog>
#include "ui_traegerdialog.h"

class TraegerDialog : public QDialog
{
	Q_OBJECT

public:
	TraegerDialog(QWidget *parent = nullptr);
	~TraegerDialog();

private:
	Ui::TraegerDialogClass ui;
};

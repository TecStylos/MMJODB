#pragma once

#include <QDialog>
#include "ui_insigniedialog.h"

class InsignieDialog : public QDialog
{
	Q_OBJECT

public:
	InsignieDialog(QWidget *parent = nullptr);
	~InsignieDialog();

private:
	Ui::InsignieDialogClass ui;
};

#include "stdafx.h"
#include "acceptcanceldialog.h"

AcceptCancelDialog::AcceptCancelDialog(QWidget *parent, const std::string& message)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.textBrowser->setText(QString::fromStdString(message));
}

AcceptCancelDialog::~AcceptCancelDialog()
{}

void AcceptCancelDialog::on_buttonBox_accepted()
{
	accept();
}

void AcceptCancelDialog::on_buttonBox_rejected()
{
	reject();
}

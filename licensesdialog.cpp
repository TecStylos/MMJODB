#include "stdafx.h"
#include "licensesdialog.h"

#include "Strings.h"

LicensesDialog::LicensesDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromStdString(STR_TITLE_LICENSES_DIALOG));
}

LicensesDialog::~LicensesDialog()
{}


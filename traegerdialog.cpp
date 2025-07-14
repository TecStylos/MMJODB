#include "stdafx.h"
#include "traegerdialog.h"

#include "Strings.h"

TraegerDialog::TraegerDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromStdString(STR_TITLE_TRAEGER_DIALOG));
}

TraegerDialog::~TraegerDialog()
{}

#include "stdafx.h"
#include "insigniedialog.h"

#include "Strings.h"

InsignieDialog::InsignieDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromStdString(STR_TITLE_INSIGNIE_DIALOG));
}

InsignieDialog::~InsignieDialog()
{}

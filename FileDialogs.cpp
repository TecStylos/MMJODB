#include "stdafx.h"
#include "FileDialogs.h"

#include "QFileDialog"

#include "Strings.h"

std::string single_file_dialog(QWidget* parent, const std::string& title, const std::string& name_filter, bool for_read_else_write)
{
	QFileDialog dialog(parent);

	// File must exist for reading, can be created for writing
	if (for_read_else_write)
		dialog.setFileMode(QFileDialog::ExistingFile);
	else
		dialog.setFileMode(QFileDialog::AnyFile);

	dialog.setNameFilter(QString::fromStdString(name_filter));

	QFileDialog::AcceptMode accept_mode = for_read_else_write ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave;
	dialog.setAcceptMode(accept_mode);
	dialog.setWindowTitle(QString::fromStdString("MMJODB: " + title));

	if (dialog.exec())
	{
		QStringList filenames = dialog.selectedFiles();
		if (filenames.size() != 1)
		{
			QErrorMessage emsg(parent);
			emsg.showMessage(QObject::tr(STR_ERR_MSG_INVALID_SELECTION), QObject::tr(STR_INT_ERR_MSG_TYPE_INVALID_SELECTION));
			return "";
		}
		return filenames[0].toStdString();
	}

	return "";
}

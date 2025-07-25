#include "stdafx.h"
#include "progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent, const std::string& title, const std::string& message)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromStdString(title));
	ui.label->setText(QString::fromStdString(message));
}

ProgressDialog::~ProgressDialog()
{
	m_thread.join();
}

void ProgressDialog::exec_progress(std::function<void()> f)
{
	m_thread = std::thread(f);
}

void ProgressDialog::setRange(int minimum, int maximum)
{
	ui.progressBar->setRange(minimum, maximum);
}

void ProgressDialog::setValue(int value)
{
	ui.progressBar->setValue(value);
}

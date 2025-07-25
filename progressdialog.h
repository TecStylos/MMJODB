#pragma once

#include <QDialog>
#include "ui_progressdialog.h"
#include <functional>

class ProgressDialog : public QDialog
{
	Q_OBJECT

public:
	ProgressDialog(QWidget *parent, const std::string& title, const std::string& message);
	~ProgressDialog();
public:
	void exec_progress(std::function<void()> f);
	void setRange(int minimum, int maximum);
	void setValue(int value);
private:
	Ui::ProgressDialogClass ui;
	std::thread m_thread;
};


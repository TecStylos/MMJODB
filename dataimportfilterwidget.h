#pragma once

#include <QWidget>
#include "ui_dataimportfilterwidget.h"

class DataImportFilterWidget : public QWidget
{
	Q_OBJECT

public:
	DataImportFilterWidget(QWidget *parent = nullptr);
	~DataImportFilterWidget();
private slots:
	void on_buttonDelete_clicked();
private:
	Ui::DataImportFilterWidgetClass ui;
};


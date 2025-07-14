#pragma once

#include <QWidget>
#include "ui_dataimportfilterwidget.h"

class DataImportFilterWidget : public QWidget
{
	Q_OBJECT

public:
	DataImportFilterWidget(QWidget *parent = nullptr);
	~DataImportFilterWidget();

private:
	Ui::DataImportFilterWidgetClass ui;
};


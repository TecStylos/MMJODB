#pragma once

#include <QWidget>
#include "ui_dataimportcolumnmappingwidget.h"

#include <vector>
#include "dataimportfilterwidget.h"

class DataImportColumnMappingWidget : public QWidget
{
	Q_OBJECT

public:
	DataImportColumnMappingWidget(QWidget *parent = nullptr);
	~DataImportColumnMappingWidget();
private slots:
	void on_buttonAddFilter_clicked();
private:
	Ui::DataImportColumnMappingWidgetClass ui;
	std::vector<DataImportFilterWidget*> m_filters;
};


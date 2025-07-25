#pragma once

#include <QWidget>
#include "ui_dataimportcolumnmappingwidget.h"

#include <vector>
#include "dataimportfilterwidget.h"

#include "CSVReader.h"

class DataImportColumnMappingWidget : public QWidget
{
	Q_OBJECT

public:
	DataImportColumnMappingWidget(QWidget *parent, const std::vector<std::string>& data_column_names, const std::string& db_column_visible_name, const std::string& db_column_type);
	~DataImportColumnMappingWidget();
public:
	void test_filters(const CSVReader& csv);
private slots:
	void on_buttonAddFilter_clicked();
private:
	Ui::DataImportColumnMappingWidgetClass ui;
};


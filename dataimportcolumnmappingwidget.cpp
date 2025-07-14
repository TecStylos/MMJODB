#include "stdafx.h"
#include "dataimportcolumnmappingwidget.h"

DataImportColumnMappingWidget::DataImportColumnMappingWidget(QWidget *parent, const std::vector<std::string>& data_column_names, const std::string& db_column_visible_name, const std::string& db_column_type)
	: QWidget(parent)
{
	ui.setupUi(this);

	for (const auto& column_name : data_column_names)
		ui.comboDataColumnName->addItem(QString::fromStdString(column_name));

	ui.labelDBColumnName->setText(QString::fromStdString(db_column_visible_name));
	ui.labelDBColumnType->setText(QString::fromStdString("(" + db_column_type + ")"));
}

DataImportColumnMappingWidget::~DataImportColumnMappingWidget()
{}

void DataImportColumnMappingWidget::on_buttonAddFilter_clicked()
{
	auto filter_widget = new DataImportFilterWidget(this);
	ui.layoutFilters->addWidget(filter_widget);
}
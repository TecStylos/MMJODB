#include "stdafx.h"
#include "dataimportcolumnmappingwidget.h"

DataImportColumnMappingWidget::DataImportColumnMappingWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

DataImportColumnMappingWidget::~DataImportColumnMappingWidget()
{}

void DataImportColumnMappingWidget::on_buttonAddFilter_clicked()
{
	auto filter_widget = new DataImportFilterWidget(this);
	ui.layoutFilters->addWidget(filter_widget);
	m_filters.push_back(filter_widget);
}
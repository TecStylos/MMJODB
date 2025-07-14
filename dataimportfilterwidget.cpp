#include "stdafx.h"
#include "dataimportfilterwidget.h"

DataImportFilterWidget::DataImportFilterWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

DataImportFilterWidget::~DataImportFilterWidget()
{}

void DataImportFilterWidget::on_buttonDelete_clicked()
{
	deleteLater();
}
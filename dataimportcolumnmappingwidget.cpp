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

void DataImportColumnMappingWidget::test_filters(const CSVReader& csv)
{
	int matches = 0;

	int col_id = ui.comboDataColumnName->currentIndex() - 1; // -1 to account for "<NULL>" option
	if (col_id < 0)
	{
		ui.labelFilterResults->setText("Keine Datensplate ausgewaehlt");
		return;
	}

	for (int i = 0; i < ui.layoutFilters->count(); ++i)
	{
		auto widget = qobject_cast<DataImportFilterWidget*>(ui.layoutFilters->itemAt(i)->widget());
		if (!widget)
			continue;

		matches += widget->test_filter(csv, col_id);
	}

	int row_count = (int)csv.get_row_count() - 1; // Exclude header row

	ui.labelFilterResults->setText("Treffer gesamt: " + QString::number(matches) + "/" + QString::number(row_count));
}

void DataImportColumnMappingWidget::on_buttonAddFilter_clicked()
{
	auto filter_widget = new DataImportFilterWidget(this);
	ui.layoutFilters->addWidget(filter_widget);
}
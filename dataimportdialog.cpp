#include "stdafx.h"
#include "dataimportdialog.h"

#include "Strings.h"
#include "FileDialogs.h"
#include "Database.h"

// TODO: Implement data import steps:
// Data mapping (data columns to database columns)
// Automatic data import (insert into database)
// Manual verification (show imported data, allow manual corrections, duplicate handling)

CSVTableModel::CSVTableModel(QObject* parent)
	: QAbstractTableModel(parent), m_p_csv(nullptr)
{
}

CSVTableModel::~CSVTableModel()
{}

int CSVTableModel::rowCount(const QModelIndex& parent) const
{
	if (m_p_csv == nullptr)
		return 0;

	return (int)m_p_csv->get_row_count();
}

int CSVTableModel::columnCount(const QModelIndex& parent) const
{
	if (m_p_csv == nullptr)
		return 0;

	return (int)m_p_csv->get_col_count();
}

QVariant CSVTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || m_p_csv == nullptr)
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		auto cell = m_p_csv->get_cell(index.row(), index.column());
		return QString::fromStdString(cell);
	}

	return QVariant();
}

Qt::ItemFlags CSVTableModel::flags(const QModelIndex& index) const
{
	if (!index.isValid() || m_p_csv == nullptr)
		return Qt::NoItemFlags;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void CSVTableModel::set_csv(CSVReader* p_csv)
{
	beginResetModel();
	m_p_csv = p_csv;
	endResetModel();
}

ConstraintListModel::ConstraintListModel(QObject* parent)
	: QAbstractListModel(parent), m_constraints(nullptr), m_current_column(-1)
{
}

ConstraintListModel::~ConstraintListModel()
{}

int ConstraintListModel::rowCount(const QModelIndex& parent) const
{
	if (m_constraints == nullptr || m_current_column < 0 || m_current_column >= (int)m_constraints->size())
		return 0;

	return (int)m_constraints->at(m_current_column).size();
}

QVariant ConstraintListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || m_constraints == nullptr || m_current_column < 0 || m_current_column >= (int)m_constraints->size())
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		auto it = m_constraints->at(m_current_column).begin();
		std::advance(it, index.row());
		return QString::fromStdString(it->first);
	}

	if (role == Qt::CheckStateRole)
	{
		auto it = m_constraints->at(m_current_column).begin();
		std::advance(it, index.row());
		return it->second ? Qt::Checked : Qt::Unchecked;
	}

	return QVariant();
}

Qt::ItemFlags ConstraintListModel::flags(const QModelIndex& index) const
{
	if (!index.isValid() || m_constraints == nullptr || m_current_column < 0 || m_current_column >= (int)m_constraints->size())
		return Qt::NoItemFlags;

	return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

bool ConstraintListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid() || m_constraints == nullptr || m_current_column < 0 || m_current_column >= (int)m_constraints->size())
		return false;

	if (role == Qt::CheckStateRole)
	{
		auto& constraints = m_constraints->at(m_current_column);
		auto it = constraints.begin();
		std::advance(it, index.row());

		it->second = (value.toInt() == Qt::Checked);
		emit dataChanged(index, index, { role });
		return true;
	}

	return false;
}

void ConstraintListModel::set_constraints(std::vector<std::map<std::string, bool>>* constraints)
{
	beginResetModel();
	m_constraints = constraints;
	m_current_column = -1; // Reset current column when constraints change
	endResetModel();
}

void ConstraintListModel::set_current_column(int col)
{
	if (col < 0 || col >= (int)m_constraints->size())
		return;
	beginResetModel();
	m_current_column = col;
	endResetModel();
}

DataImportDialog::DataImportDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromStdString(STR_TITLE_DATA_IMPORT_DIALOG));

	ui.tableViewReadData->setModel(&m_csv_model);
	ui.tableViewReadData->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);

	m_constraints_model.set_constraints(&m_constraints);
	ui.listConstraints->setModel(&m_constraints_model);

	setup_tab_mappings();

	hide_all_tabs_except_current();
	update_prev_next_done_visibility();
}

DataImportDialog::~DataImportDialog()
{}

void DataImportDialog::setup_tab_mappings()
{
	for (auto& widget : m_column_mapping_widgets)
	{
		ui.layoutColumnMappings->removeWidget(widget.second);
		delete widget.second;
	}
	m_column_mapping_widgets.clear();

	std::vector<std::string> data_column_names;
	data_column_names.push_back("<NULL>");
	for (size_t i = 0; i < m_csv.get_col_count(); ++i)
		data_column_names.push_back(m_csv.get_cell(0, i));

	auto db = Database::get_instance();
	for (auto& table_name : db->get_table_names())
	{
		auto table_info = db->get_table_info(table_name);
		for (auto& column : table_info.columns)
		{
			auto entry_name = table_name + "." + column.first;

			auto widget = new DataImportColumnMappingWidget(
				ui.tabMapping,
				data_column_names,
				entry_name,
				column_type_to_string(column.second)
			);

			ui.layoutColumnMappings->addWidget(widget);
			m_column_mapping_widgets[entry_name] = widget;
		}
	}
}

void DataImportDialog::hide_all_tabs_except_current()
{
	int curr_index = ui.tabsImportSteps->currentIndex();
	for (int i = 0; i < ui.tabsImportSteps->count(); ++i)
		ui.tabsImportSteps->setTabVisible(i, i == curr_index);
}

void DataImportDialog::update_prev_next_done_visibility()
{
	int curr_index = ui.tabsImportSteps->currentIndex();
	ui.buttonPreviousStep->setVisible(curr_index > 0);
	ui.buttonNextStep->setVisible(curr_index < ui.tabsImportSteps->count() - 1);
	ui.buttonDone->setVisible(curr_index == ui.tabsImportSteps->count() - 1);
}

void DataImportDialog::load_constraints_from_imported_csv()
{
	m_constraints.clear();

	if (m_csv.get_row_count() < 2)
	{
		m_emsg.showMessage(
			tr("Die CSV-Datei muss mindestens eine Datenzeile enthalten."),
			tr(STR_INT_ERR_TYPE_CSV_READER)
		);
		return;
	}

	m_constraints.resize(m_csv.get_col_count());

	for (size_t col = 0; col < m_csv.get_col_count(); ++col)
	{
		auto& constraints = m_constraints[col];
		constraints.clear();
		for (size_t row = 1; row < m_csv.get_row_count(); ++row)
		{
			auto& cell = m_csv.get_cell(row, col);
			constraints[cell] = true; // Mark this value as valid
		}
	}
}

void DataImportDialog::update_constraint_list()
{
	int current_column = ui.listColumnsToImport->currentRow();
	if (current_column < 0 || current_column >= ui.listColumnsToImport->count())
		return;
	// Update constraints model for the selected column
	ui.listConstraints->clearSelection();
	m_constraints_model.set_current_column(current_column);
	// Select the first item in the constraints list
	if (m_constraints_model.rowCount() > 0)
		ui.listConstraints->setCurrentIndex(m_constraints_model.index(0));
}

void DataImportDialog::on_buttonOpenFile_clicked()
{
	// File selection (*.csv)
	auto filepath = single_file_dialog(this, STR_DIALOG_IMPORT_DATA_TITLE, STR_DIALOG_IMPORT_DATA_FILTER, true);
	if (filepath.empty())
		return;

	// Read CSV file
	try
	{
		// Update csv model with new data
		m_csv_model.set_csv(nullptr);
		m_csv = CSVReader(filepath);
		m_csv_model.set_csv(&m_csv);
	}
	catch (const std::runtime_error& e)
	{
		m_emsg.showMessage(
			tr(e.what()),
			tr(STR_INT_ERR_TYPE_CSV_READER)
		);
		return;
	}

	setWindowTitle(QString::fromStdString(STR_TITLE_DATA_IMPORT_DIALOG + (" - " + filepath)));

	// Update list containing columns to import
	ui.listColumnsToImport->clear();
	for (size_t i = 0; i < m_csv.get_col_count(); ++i)
	{
		auto item = new QListWidgetItem(QString::fromStdString(m_csv.get_cell(0, i)));
		ui.listColumnsToImport->addItem(item);
	}
	ui.listColumnsToImport->setCurrentRow(0);

	// Update constraints
	load_constraints_from_imported_csv();

	// Update constraints list for selected column
	update_constraint_list();

	// Setup tab mappings
	setup_tab_mappings();
}

void DataImportDialog::on_buttonPreviousStep_clicked()
{
	int curr_index = ui.tabsImportSteps->currentIndex();
	if (curr_index > 0)
	{
		ui.tabsImportSteps->setCurrentIndex(curr_index - 1);
		hide_all_tabs_except_current();
		update_prev_next_done_visibility();
	}
}

void DataImportDialog::on_buttonNextStep_clicked()
{
	int curr_index = ui.tabsImportSteps->currentIndex();
	if (curr_index < ui.tabsImportSteps->count() - 1)
	{
		ui.tabsImportSteps->setCurrentIndex(curr_index + 1);
		hide_all_tabs_except_current();
		update_prev_next_done_visibility();
	}
}

void DataImportDialog::on_listColumnsToImport_currentRowChanged(int currentRow)
{
	if (currentRow < 0 || currentRow >= ui.listColumnsToImport->count())
		return;
	// Update constraints list for selected column
	m_constraints_model.set_current_column(currentRow);
	update_constraint_list();
}
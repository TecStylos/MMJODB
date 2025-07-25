#include "stdafx.h"
#include "dataimportdialog.h"

#include "acceptcanceldialog.h"

#include "Strings.h"
#include "FileDialogs.h"
#include "Database.h"

// TODO: Implement data import steps:
// Data mapping (data columns to database columns)
// Automatic data import (insert into database)
// Manual verification (show imported data, allow manual corrections, duplicate handling)

CSVTableModel::CSVTableModel(QObject* parent)
	: QAbstractTableModel(parent), m_p_csv(nullptr), m_is_editable(false)
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
		if (!cell.has_value())
			return QVariant();

		return QString::fromStdString(cell.value());
	}

	return QVariant();
}

bool CSVTableModel::setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)
{
	bool success = m_p_csv->set_cell(index.row(), index.column(), value.toString().toStdString());

	if (!success)
		return false;

	dataChanged(index, index);
	return true;
}

Qt::ItemFlags CSVTableModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags;

	if (!index.isValid() || m_p_csv == nullptr)
		return flags;

	flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if (m_is_editable)
		flags |= Qt::ItemIsEditable;

	return flags;
}

void CSVTableModel::set_csv(CSVReader* p_csv)
{
	beginResetModel();
	m_p_csv = p_csv;
	endResetModel();
}

void CSVTableModel::set_editable(bool is_editable)
{
	m_is_editable = is_editable;
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
	update_button_visibility();
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
		data_column_names.push_back(m_csv.get_cell(0, i).value());

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

void DataImportDialog::update_button_visibility()
{
	int curr_index = ui.tabsImportSteps->currentIndex();

	ui.buttonPreviousStep->setVisible(curr_index == 1);
	ui.buttonNextStep->setVisible(curr_index == 0);
	ui.buttonImportData->setVisible(curr_index == 1);
	ui.buttonDone->setVisible(curr_index == 2);
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
			auto cell = m_csv.get_cell(row, col).value();
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

void DataImportDialog::auto_import_data()
{
	m_import_status_per_row.clear();
	m_import_status_per_row.resize(m_csv.get_row_count(), ImportStatus::None);

	// TODO: Implement working progress bar
	//auto dialog = new ProgressDialog(this, "MMJODB - Daten Import", "Daten werden importiert...");
	//dialog->setRange(0, m_csv.get_row_count() - 1);
	//dialog->setValue(0);
	//dialog->open();

	auto db = Database::get_instance();

	for (size_t row_id = 1; row_id < m_csv.get_row_count(); ++row_id)
	{
		auto import_status = ImportStatus::NeedsCorrection;
		
		auto query = db->make_query("INSERT INTO traeger; INSERT INTO insignie; INSERT INTO verleihung;");

		// TODO: Bind values to import

		if (!query.execute(nullptr))
			import_status = ImportStatus::Imported;

		m_import_status_per_row[row_id] = import_status;

		//dialog->setValue(row_id);
	}

	//dialog->close();
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
		auto item = new QListWidgetItem(QString::fromStdString(m_csv.get_cell(0, i).value()));
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
		update_button_visibility();
	}
}

void DataImportDialog::on_buttonNextStep_clicked()
{
	int curr_index = ui.tabsImportSteps->currentIndex();
	if (curr_index < ui.tabsImportSteps->count() - 1)
	{
		ui.tabsImportSteps->setCurrentIndex(curr_index + 1);
		hide_all_tabs_except_current();
		update_button_visibility();
	}
}

void DataImportDialog::on_buttonImportData_clicked()
{
	AcceptCancelDialog dialog(this, "Wollen Sie die Daten importieren?\nDieser Schritt kann nicht rueckgaengig gemacht werden.");

	if (!dialog.exec())
		return;

	auto_import_data();

	on_buttonNextStep_clicked();
}

void DataImportDialog::on_buttonDone_clicked()
{
	AcceptCancelDialog dialog(this, "Wollen Sie den Daten-Import beenden?\nDieser Schritt kann nicht rueckgaengig gemacht werden.");

	if (!dialog.exec())
		return;

	close();
}

void DataImportDialog::on_listColumnsToImport_currentRowChanged(int currentRow)
{
	if (currentRow < 0 || currentRow >= ui.listColumnsToImport->count())
		return;
	// Update constraints list for selected column
	m_constraints_model.set_current_column(currentRow);
	update_constraint_list();
}

void DataImportDialog::on_buttonTestFilters_clicked()
{
	for (int i = 0; i < ui.layoutColumnMappings->count(); ++i)
	{
		auto widget = qobject_cast<DataImportColumnMappingWidget*>(ui.layoutColumnMappings->itemAt(i)->widget());
		if (!widget)
			continue;

		widget->test_filters(m_csv);
	}
}
#pragma once

#include <QDialog>
#include "ui_dataimportdialog.h"

#include "CSVReader.h"

#include "dataimportcolumnmappingwidget.h"
#include "progressdialog.h"

class CSVTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	CSVTableModel(QObject* parent = nullptr);
	virtual ~CSVTableModel();
public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
public:
	void set_csv(CSVReader* p_csv);
	void set_editable(bool is_editable);
private:
	CSVReader* m_p_csv;
	bool m_is_editable;
};

class ConstraintListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ConstraintListModel(QObject* parent = nullptr);
	virtual ~ConstraintListModel();
public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
public:
	void set_constraints(std::vector<std::map<std::string, bool>>* constraints);
	void set_current_column(int col);
private:
	std::vector<std::map<std::string, bool>>* m_constraints;
	int m_current_column;
};

class DataImportDialog : public QDialog
{
	Q_OBJECT

private:
	enum class ImportStatus
	{
		None,
		Imported,
		Duplicate,
		NeedsCorrection
	};
public:
	DataImportDialog(QWidget *parent = nullptr);
	~DataImportDialog();
private:
	void setup_tab_mappings();
	void hide_all_tabs_except_current();
	void update_button_visibility();
	void load_constraints_from_imported_csv();
	void update_constraint_list();
	void auto_import_data();
private slots:
	void on_buttonOpenFile_clicked();
	void on_buttonPreviousStep_clicked();
	void on_buttonNextStep_clicked();
	void on_buttonImportData_clicked();
	void on_buttonDone_clicked();
	void on_listColumnsToImport_currentRowChanged(int currentRow);
	void on_buttonTestFilters_clicked();
private:
	Ui::DataImportDialogClass ui;
	QErrorMessage m_emsg;
	CSVReader m_csv;
	std::vector<ImportStatus> m_import_status_per_row;
	CSVTableModel m_csv_model;
	std::vector<std::map<std::string, bool>> m_constraints;
	ConstraintListModel m_constraints_model;
	std::map<std::string, DataImportColumnMappingWidget*> m_column_mapping_widgets;
};


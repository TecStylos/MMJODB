#include "stdafx.h"
#include "mmjodb.h"

#include "Database.h"

MMJODB::MMJODB(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

MMJODB::~MMJODB()
{}

void MMJODB::open_database(const std::string& filepath, bool allow_create)
{
    try
    {
        auto db = std::make_shared<Database>(filepath, allow_create);
        Database::set_instance(db);
    }
    catch (std::runtime_error& e)
    {
        m_emsg.showMessage(tr(e.what()), tr("Fehler"));
    }
}

bool MMJODB::warn_if_no_db_open(std::shared_ptr<Database> db)
{
    if (db != nullptr)
        return false;

    m_emsg.showMessage(tr("Keine Datenbank geoeffnet"), tr("Fehler"));

    return true;
}

void MMJODB::on_actionDatenbankOeffnen_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Datenbanken (*.mmjodb)"));
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    dialog.setWindowTitle(tr("Datenbank oeffnen"));

    if (dialog.exec())
    {
        QStringList filenames = dialog.selectedFiles();
        if (filenames.size() != 1)
        {
            m_emsg.showMessage(tr("Ungueltige Auswahl"), tr("Fehler"));
        }
        else
        {
            open_database(filenames[0].toStdString(), false);

            // Create necessary tables
        }
    }
}

void MMJODB::on_actionDatenbankNeu_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Datenbanken (*.mmjodb)"));
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    dialog.setWindowTitle(tr("Neue Datenbank anlegen"));

    if (dialog.exec())
    {
        QStringList filenames = dialog.selectedFiles();
        if (filenames.size() != 1)
            m_emsg.showMessage(tr("Ungueltige Auswahl"), tr("Fehler"));
        else
            open_database(filenames[0].toStdString(), true);
    }
}

void MMJODB::on_buttonRunSQLQuery_clicked()
{
    auto db = Database::get_instance();
    if (warn_if_no_db_open(db))
        return;

    auto sql_stmt_str = ui.textEditSQLQuery->toPlainText().toStdString();

    auto query = db->make_query(sql_stmt_str);

    ui.labelSQLError->clear();
    ui.tabSQLOutput->clear();

    int num_tabs = 0;

    QTableWidget* curr_table = nullptr;

    auto table_callback = [&](const std::vector<std::string>& col_names)
        {
			curr_table = new QTableWidget();
            curr_table->setColumnCount(col_names.size());

            // Set table header labels
            QStringList col_names_qstr;
            col_names_qstr.reserve(col_names.size());
            for (const auto& col_name : col_names)
                col_names_qstr.append(QString::fromStdString(col_name));
            
            curr_table->setHorizontalHeaderLabels(col_names_qstr);

			// Put table in scroll area
			auto scroll_area = new QScrollArea();
			scroll_area->setWidget(curr_table);
			scroll_area->setWidgetResizable(true);
			scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
			scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

            // Create new tab for SQL output table
			std::string tab_name = "Ausgabe " + std::to_string(++num_tabs);
			ui.tabSQLOutput->addTab(scroll_area, QString::fromStdString(tab_name));

            return true;
        };

    auto row_callback = [&](const DBRow& row)
        {
			// Add row to table
			int row_index = curr_table->rowCount();
            curr_table->insertRow(row_index);

            for (int i = 0; i < row.get_col_cnt(); ++i)
            {
                auto col = row.get_col(i, DBRow::Column::Type::Text);
                auto item = new QTableWidgetItem(QString::fromStdString(col.value().get_text()));
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                curr_table->setItem(row_index, i, item);
            }
            return true;
        };

    if (query.execute(row_callback, table_callback))
        ui.labelSQLError->setText(
            QString::fromStdString(
                "Fehler: " + query.get_err_msg()
            )
        );
}
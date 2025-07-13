#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mmjodb.h"

#include "Database.h"

class MMJODB : public QMainWindow
{
    Q_OBJECT
public:
    MMJODB(QWidget *parent = nullptr);
    ~MMJODB();
private:
    void open_database(const std::string& filepath, bool allow_create);
    bool warn_if_no_db_open(std::shared_ptr<Database> db);
private slots:
    void on_actionOpenDatabase_triggered();
    void on_actionNewDatabase_triggered();
    void on_actionShowDetails_triggered();
    void on_actionImportData_triggered();
    void on_actionImportImages_triggered();
    void on_actionShowLicenses_triggered();
    void on_actionOpenWebsite_triggered();
    void on_buttonRunSQLQuery_clicked();
private:
    Ui::MMJODBClass ui;
    QErrorMessage m_emsg;
};
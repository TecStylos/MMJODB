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
    void on_actionDatenbankOeffnen_triggered();
    void on_actionDatenbankNeu_triggered();
    void on_buttonRunSQLQuery_clicked();
    void on_actionLizenzen_triggered();
    void on_actionWebseite_triggered();
private:
    Ui::MMJODBClass ui;
    QErrorMessage m_emsg;
};
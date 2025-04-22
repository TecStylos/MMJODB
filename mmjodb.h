#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mmjodb.h"

class MMJODB : public QMainWindow
{
    Q_OBJECT

public:
    MMJODB(QWidget *parent = nullptr);
    ~MMJODB();

private:
    Ui::MMJODBClass ui;
};

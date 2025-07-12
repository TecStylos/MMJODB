#include "stdafx.h"
#include "mmjodb.h"
#include <QtWidgets/QApplication>

#include "Database.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MMJODB w;
    w.show();
    return a.exec();
}

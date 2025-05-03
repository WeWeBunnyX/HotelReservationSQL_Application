#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>
#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    sqlDriverCheck();
    testDatabaseConnection();

    MainWindow w;
    w.show();

    return a.exec();
}

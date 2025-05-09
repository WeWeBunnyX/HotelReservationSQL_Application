#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>
#include "database.h"
#include "mainwindow.h"
#include "loginscreen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    sqlDriverCheck();
    testDatabaseConnection();

    // MainWindow w;
    // w.show();

    LoginScreen l;
    l.show();
    return a.exec();
}

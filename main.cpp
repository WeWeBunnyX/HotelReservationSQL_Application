#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>
#include "database.h"
#include "mainwindow.h"
#include "loginscreen.h"
#include "registerscreen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    sqlDriverCheck();
    setDatabaseConnection();

    // MainWindow w;
    // w.show();

    // LoginScreen l;
    // l.show();

    RegisterScreen r;
    r.show();

    return a.exec();
}

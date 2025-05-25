#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>
#include "database.h"
#include "loginscreen.h"

// #include "mainwindow.h"
// #include "loginscreen.h"
// #include "registerscreen.h"
#include "dashboard.h"
// #include "reservations.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    sqlDriverCheck();
    setDatabaseConnection();

    LoginScreen login;
    login.show();

    // MainWindow w;
    // w.show();

    // LoginScreen l;
    // l.show();

    //RegisterScreen r;
    //r.show();

    // Dashboard d;
    // d.show();

    // Reservations r;
    // r.show();

    return a.exec();
}

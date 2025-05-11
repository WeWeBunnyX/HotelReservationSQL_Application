#include "database.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

void setDatabaseConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "main_connection");

    db.setHostName("localhost");
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("bahria123");

    attemptDatabaseConnection();


}

void attemptDatabaseConnection(){
     QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open()) {
        qDebug() << "Database connection failed! ❌";
        QMessageBox::critical(nullptr, "Connection Failed", "❌ Unable to connect to the database!");
        qDebug() << db.lastError().text();

    } else {
        qDebug() << " Database connection successful! ✅";
        QMessageBox::information(nullptr, "Connection", "✅ Database connection successful!");
        db.open();
    }
}



void sqlDriverCheck(){
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "\nAvailable SQL drivers:";
    for (const QString &driver : drivers) {
        qDebug() << driver;
    }

    if (!drivers.contains("QPSQL")) {
        qDebug() << "Postgre SQL Driver Available!";
    } else {
        qDebug() << "\nPostgre SQL Driver Available!";
    }

}

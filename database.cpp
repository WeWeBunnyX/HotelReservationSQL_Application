#include "database.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

void testDatabaseConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("localhost");
    db.setDatabaseName("hotel_test");
    db.setUserName("qtuser");
    db.setPassword("doubleh");

    if (!db.open()) {
        qDebug() << "Database connection failed! ❌";
        QMessageBox::critical(nullptr, "Connection Failed", "❌ Unable to connect to the database!");
        qDebug() << db.lastError().text();

    } else {
        qDebug() << " Database connection successful! ✅";
        QMessageBox::information(nullptr, "Connection", "✅ Database connection successful!");
        db.close();
    }
}


void sqlDriverCheck(){
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "\nAvailable SQL drivers:";
    for (const QString &driver : drivers) {
        qDebug() << driver;
    }

    if (!drivers.contains("QMYSQL")) {
        qDebug() << "MySQL/MariaDB driver is not available!";
    } else {
        qDebug() << "\nMySQL/MariaDB driver is available!";
    }

}

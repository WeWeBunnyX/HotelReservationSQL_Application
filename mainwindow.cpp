#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dashboard.h"
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dashboard(nullptr)
    , customersModule(nullptr)
    , roomsModule(nullptr)
    , paymentsModule(nullptr)
    , reportsModule(nullptr)
{
    ui->setupUi(this);
    connect(ui->welcomeLabel, &QLabel::linkActivated, this, &MainWindow::onLinkActivated);
}

MainWindow::~MainWindow()
{
    delete dashboard;
    delete customersModule;
    delete roomsModule;
    delete paymentsModule;
    delete reportsModule;
    delete ui;
}

void MainWindow::onLinkActivated(const QString &link)
{
    if (link == "license") {
        QDesktopServices::openUrl(QUrl("https://github.com/WeWeBunnyX/HotelReservationSQL_Application/blob/master/LICENSE"));
    } else if (link == "github") {
        QDesktopServices::openUrl(QUrl("https://github.com/WeWeBunnyX/HotelReservationSQL_Application"));
    } else if (link == "start") {
        if (!dashboard) {
            dashboard = new Dashboard(this);
        }
        dashboard->show();
        hide();
    }
}

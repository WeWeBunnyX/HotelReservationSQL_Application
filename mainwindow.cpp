#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dashboard.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLinkActivated(const QString &link)
{
    if (link == "license") {
        QDesktopServices::openUrl(QUrl("https://github.com/yourusername/HotelReservationSQL_Application/blob/main/LICENSE"));
    } else if (link == "github") {
        QDesktopServices::openUrl(QUrl("https://github.com/yourusername/HotelReservationSQL_Application"));
    } else if (link == "start") {
        Dashboard *dashboard = new Dashboard(this);
        dashboard->show();
        hide();
    }
}

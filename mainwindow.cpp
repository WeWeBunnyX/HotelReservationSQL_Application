#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customers.h"
#include "rooms.h"
#include "payments.h"
#include "reports.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btn_customers, &QPushButton::clicked, this, &MainWindow::openCustomers);
    connect(ui->btn_rooms, &QPushButton::clicked, this, &MainWindow::openRooms);
    connect(ui->btn_payments, &QPushButton::clicked, this, &MainWindow::openPayments);
    connect(ui->btn_reports, &QPushButton::clicked, this, &MainWindow::openReports);

    customersWindow = nullptr;
    roomsWindow = nullptr;
    paymentsWindow = nullptr;
    reportsWindow = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openCustomers()
{
    if (!customersWindow)
        customersWindow = new CustomersModule(this);
    customersWindow->show();
    customersWindow->raise();
    customersWindow->activateWindow();
}

void MainWindow::openRooms()
{
    if (!roomsWindow)
        roomsWindow = new RoomsModule(this);
    roomsWindow->show();
    roomsWindow->raise();
    roomsWindow->activateWindow();
}

void MainWindow::openPayments()
{
    if (!paymentsWindow)
        paymentsWindow = new PaymentsModule(this);
    paymentsWindow->show();
    paymentsWindow->raise();
    paymentsWindow->activateWindow();
}

void MainWindow::openReports()
{
    if (!reportsWindow)
        reportsWindow = new ReportsModule(this);
    reportsWindow->show();
    reportsWindow->raise();
    reportsWindow->activateWindow();
}

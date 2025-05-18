#include "dashboard.h"
#include "ui_dashboard.h"
#include "customers.h"
#include <QDebug>

Dashboard::Dashboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard),
    customersModule(new CustomersModule(this))  // Initialize CustomersModule
{
    ui->setupUi(this);

    ui->fullMenuWidget->setVisible(true);
    ui->iconOnlyWidget->setVisible(false);

    // Connect menu toggle buttons
    connect(ui->homeButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->homeButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->reservationButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->reservationButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->customerButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->customerButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->roomButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->roomButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->paymentButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->paymentButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->reportButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->reportButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->settingButton_1, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);
    connect(ui->settingButton_2, &QPushButton::toggled, this, &Dashboard::onMenuButtonToggled);

    // Add the CustomersModule to the stacked widget
    ui->stackedWidget->insertWidget(2, customersModule);  // Ensure this is index 2 as per your logic

    ui->stackedWidget->setCurrentIndex(0);  // Default to Home Page
}

Dashboard::~Dashboard()
{
    delete ui;
}

void Dashboard::onMenuButtonToggled(bool checked)
{
    if (!checked) return;

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    qDebug() << "Button toggled:" << button->objectName() << "Checked:" << checked;

    if (button == ui->homeButton_1 || button == ui->homeButton_2) {
        ui->stackedWidget->setCurrentIndex(0); // Home Page
    } else if (button == ui->reservationButton_1 || button == ui->reservationButton_2) {
        ui->stackedWidget->setCurrentIndex(1); // Reservations Page
    } else if (button == ui->customerButton_1 || button == ui->customerButton_2) {
        ui->stackedWidget->setCurrentIndex(2); // Customers Page (Custom Widget)
    } else if (button == ui->roomButton_1 || button == ui->roomButton_2) {
        ui->stackedWidget->setCurrentIndex(3); // Room Page
    } else if (button == ui->paymentButton_1 || button == ui->paymentButton_2) {
        ui->stackedWidget->setCurrentIndex(4); // Payment Page
    } else if (button == ui->reportButton_1 || button == ui->reportButton_2) {
        ui->stackedWidget->setCurrentIndex(5); // Reports Page
    } else if (button == ui->settingButton_1 || button == ui->settingButton_2) {
        ui->stackedWidget->setCurrentIndex(6); // Settings Page
    }
}

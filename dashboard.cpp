#include "dashboard.h"
#include "ui_dashboard.h"
#include <QDebug>

Dashboard::Dashboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard),
    customersModule(new CustomersModule(this)),
    paymentsModule(new PaymentsModule(this))
{
    ui->setupUi(this);

    // Add module widgets to the stackedWidget
    ui->stackedWidget->insertWidget(2, customersModule);
    ui->stackedWidget->insertWidget(4, paymentsModule);

    // Sidebar button toggles
    ui->fullMenuWidget->setVisible(true);
    ui->iconOnlyWidget->setVisible(false);

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

    // Default page
    ui->stackedWidget->setCurrentIndex(0);
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
        ui->stackedWidget->setCurrentIndex(0);
    } else if (button == ui->reservationButton_1 || button == ui->reservationButton_2) {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (button == ui->customerButton_1 || button == ui->customerButton_2) {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (button == ui->roomButton_1 || button == ui->roomButton_2) {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (button == ui->paymentButton_1 || button == ui->paymentButton_2) {
        ui->stackedWidget->setCurrentIndex(4);
    } else if (button == ui->reportButton_1 || button == ui->reportButton_2) {
        ui->stackedWidget->setCurrentIndex(5);
    } else if (button == ui->settingButton_1 || button == ui->settingButton_2) {
        ui->stackedWidget->setCurrentIndex(6);
    }
}

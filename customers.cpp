#include "customers.h"
#include "ui_customers.h"

CustomersModule::CustomersModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomersModule)
{
    ui->setupUi(this);

    ui->customersTable->setColumnWidth(0, 200);
    ui->customersTable->setColumnWidth(1, 200);
    ui->customersTable->setColumnWidth(2, 200);
    ui->customersTable->setColumnWidth(3, 170);
    ui->customersTable->setColumnWidth(4, 170);
}

CustomersModule::~CustomersModule()
{
    delete ui;
}

#include "customers.h"
#include "ui_customers.h"

CustomersModule::CustomersModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomersModule)
{
    ui->setupUi(this);
}

CustomersModule::~CustomersModule()
{
    delete ui;
}
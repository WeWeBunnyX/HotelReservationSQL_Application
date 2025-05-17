#include "payments.h"
#include "ui_payments.h"

PaymentsModule::PaymentsModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaymentsModule)
{
    ui->setupUi(this);
}

PaymentsModule::~PaymentsModule()
{
    delete ui;
}
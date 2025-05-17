#include "reports.h"
#include "ui_reports.h"

ReportsModule::ReportsModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportsModule)
{
    ui->setupUi(this);
}

ReportsModule::~ReportsModule()
{
    delete ui;
}
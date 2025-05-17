#include "rooms.h"
#include "ui_rooms.h"

RoomsModule::RoomsModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoomsModule)
{
    ui->setupUi(this);
}

RoomsModule::~RoomsModule()
{
    delete ui;
}
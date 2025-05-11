#include "registerscreen.h"
#include "ui_registerscreen.h"

RegisterScreen::RegisterScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterScreen)
{
    ui->setupUi(this);
}

RegisterScreen::~RegisterScreen()
{
    delete ui;
}

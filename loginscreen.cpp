#include "loginscreen.h"
#include "ui_loginscreen.h"

LoginScreen::LoginScreen(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginScreen)
{
    ui->setupUi(this);
}

LoginScreen::~LoginScreen()
{
    delete ui;
}

void LoginScreen::on_LoginButton_clicked()
{
   qDebug() << "Login Button Clicked";
}

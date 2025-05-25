#include "loginscreen.h"
#include "ui_loginscreen.h"
#include "dashboard.h" 
#include <QDebug>

LoginScreen::LoginScreen(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginScreen)
    , registerScreen(nullptr)
{
    ui->setupUi(this);
    connect(ui->RegisterButton, &QPushButton::clicked, this, &LoginScreen::on_RegisterButton_clicked);
}

LoginScreen::~LoginScreen()
{
    delete ui;
    delete registerScreen;
}

void LoginScreen::on_LoginButton_clicked()
{
    qDebug() << "Login Button Clicked";


    QString username = ui->username->text().trimmed();
    QString password = ui->lineEdit_2->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password is empty.";
        return;
    }

    qDebug() << "Login successful. Opening Dashboard.";

    Dashboard *dashboard = new Dashboard();
    dashboard->show();

    this->close();
}

void LoginScreen::on_RegisterButton_clicked()
{
    if (!registerScreen) {
        registerScreen = new RegisterScreen(this);
    }

    if (registerScreen->exec() == QDialog::Accepted) {
        qDebug() << "User registered successfully. Returning to login screen.";
    }
}



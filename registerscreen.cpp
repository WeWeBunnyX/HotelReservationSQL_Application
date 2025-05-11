#include "registerscreen.h"
#include "ui_registerscreen.h"
#include "database.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>


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



void RegisterScreen::on_registerButton_clicked()
{
    registerUser();
}



void RegisterScreen::registerUser()
{
    QString username = ui->usernameField->text().trimmed();
    QString email = ui->emailField->text().trimmed();
    QString contact = ui->contactField->text().trimmed();
    QString password = ui->passwordField->text();
    QString confirmPassword = ui->confPasswordField->text();

    if (username.isEmpty() || email.isEmpty() || contact.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Password Mismatch", "Passwords do not match.");
        return;
    }

    applyUserRegistrationQuery(username, email, contact, password);
}




void RegisterScreen::applyUserRegistrationQuery(const QString &username, const QString &email,
                                                const QString &contact, const QString &password)
{
    QSqlQuery query(QSqlDatabase::database("main_connection"));

    query.prepare("INSERT INTO Users (Full_Name, Email, Phone_Number, Password) "
                  "VALUES (:full_name, :email, :phone, :password)");
    query.bindValue(":full_name", username);
    query.bindValue(":email", email);
    query.bindValue(":phone", contact);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "User registered successfully.");
        this->accept();
    }
}


void RegisterScreen::on_cancelButton_clicked(){
    this->close();
}


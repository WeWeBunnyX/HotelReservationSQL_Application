#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QDialog>
#include "registerscreen.h"

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QDialog
{
    Q_OBJECT

public:
    explicit LoginScreen(QWidget *parent = nullptr);
    ~LoginScreen();

private slots:
    void on_LoginButton_clicked();
    void on_RegisterButton_clicked(); 

private:
    Ui::LoginScreen *ui;
    RegisterScreen *registerScreen; 
};

#endif // LOGINSCREEN_H

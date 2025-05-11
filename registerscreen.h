#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QDialog>

namespace Ui {
class RegisterScreen;
}

class RegisterScreen : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterScreen(QWidget *parent = nullptr);
    ~RegisterScreen();

private slots:
    void registerUser();
    void on_registerButton_clicked();
    void on_cancelButton_clicked();

private:
    void applyUserRegistrationQuery(const QString &username, const QString &email,
                                    const QString &contact, const QString &password);

    Ui::RegisterScreen *ui;
};

#endif // REGISTERSCREEN_H

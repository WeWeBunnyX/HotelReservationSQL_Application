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

private:
    Ui::RegisterScreen *ui;
};

#endif // REGISTERSCREEN_H

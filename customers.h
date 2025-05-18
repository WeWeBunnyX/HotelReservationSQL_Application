#ifndef CUSTOMERS_H
#define CUSTOMERS_H

#include <QWidget>

namespace Ui {
class CustomersModule;
}

class CustomersModule : public QWidget
{
    Q_OBJECT

public:
    explicit CustomersModule(QWidget *parent = nullptr);
    ~CustomersModule();

private:
    Ui::CustomersModule *ui;
};

#endif // CUSTOMERS_H

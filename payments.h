#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <QWidget>

namespace Ui {
class PaymentsModule;
}

class PaymentsModule : public QWidget
{
    Q_OBJECT

public:
    explicit PaymentsModule(QWidget *parent = nullptr);
    ~PaymentsModule();

private:
    Ui::PaymentsModule *ui;
};

#endif // PAYMENTS_H
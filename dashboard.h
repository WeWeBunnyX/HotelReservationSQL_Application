#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include "customers.h"
#include "payments.h"

namespace Ui {
class Dashboard;
}

class Dashboard : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard(QWidget *parent = nullptr);
    ~Dashboard();

private slots:
    void onMenuButtonToggled(bool checked);

private:
    Ui::Dashboard *ui;
    CustomersModule *customersModule;
    PaymentsModule *paymentsModule;
};

#endif

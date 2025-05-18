#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QPushButton>
#include "customers.h"  // Include CustomersModule

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
    void onMenuButtonToggled(bool checked);  // Handles all toggle buttons

private:
    Ui::Dashboard *ui;
    CustomersModule *customersModule;  // Customers module pointer
};

#endif // DASHBOARD_H

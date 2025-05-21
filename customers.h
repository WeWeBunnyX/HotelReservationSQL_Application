#ifndef CUSTOMERSMODULE_H
#define CUSTOMERSMODULE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
class CustomersModule;
}

class CustomersModule : public QWidget
{
    Q_OBJECT

public:
    explicit CustomersModule(QWidget *parent = nullptr);
    ~CustomersModule();

private slots:
    void onSearchButtonClicked();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();

private:
    Ui::CustomersModule *ui;
    QSqlDatabase m_db;
    QSqlQueryModel *m_model;
    void loadCustomers(const QString &searchTerm = "");
};

#endif // CUSTOMERSMODULE_H

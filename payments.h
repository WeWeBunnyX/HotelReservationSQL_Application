#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
class PaymentsModule;
}

class PaymentsModule : public QWidget
{
    Q_OBJECT

public:
    explicit PaymentsModule(QWidget *parent = nullptr);
    ~PaymentsModule();

private slots:
    void on_buttonAdd_clicked();
    void on_buttonUpdate_clicked();
    void on_buttonDelete_clicked();
    void on_buttonClear_clicked();
    void on_lineEditSearch_textChanged(const QString &text);
    void on_tablePayments_clicked(const QModelIndex &index);

private:
    Ui::PaymentsModule *ui;
    QSqlDatabase m_db;
    QSqlQueryModel *m_model;
    bool m_connectionsSetup = false;

    void loadPayments(const QString &filter = QString());
    void populateReservationComboBox();
    void clearForm();
    bool validateForm(int &reservationId, double &amount, QString &paymentStatus, QString &method, QDate &date, QString &transactionId, QString &notes);
    void setupConnections();
};

#endif // PAYMENTS_H

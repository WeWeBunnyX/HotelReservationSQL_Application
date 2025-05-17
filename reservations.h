#ifndef RESERVATIONS_H
#define RESERVATIONS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include "ui_reservations.h"


QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class Reservations : public QWidget
{
    Q_OBJECT

public:
    explicit Reservations(QWidget *parent = nullptr);
    ~Reservations();

private slots:
    void on_btnAdd_clicked();
    void on_btnUpdate_clicked();
    void on_btnDelete_clicked();
    void on_btnClear_clicked();
    void on_btnRefresh_clicked();
    void on_tableViewReservations_clicked(const QModelIndex &index);

private:
    Ui::Reservations *ui;
    QSqlDatabase m_db;
    QSqlTableModel *m_model;

    void loadUsers();
    void loadRooms();
    void loadReservations();
    void clearForm();
    void populateForm(int row);
};

#endif // RESERVATIONS_H

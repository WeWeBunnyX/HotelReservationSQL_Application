#include "reservations.h"
#include "ui_reservations.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QDate>

Reservations::Reservations(QWidget *parent)
    : QWidget(parent), ui(new Ui::Reservations)
{
    ui->setupUi(this);

    m_db = QSqlDatabase::database("main_connection");

    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }


    m_model = new QSqlTableModel(this, m_db);
    m_model->setTable("reservations");
    m_model->select();

    ui->tableViewReservations->setModel(m_model);
    ui->tableViewReservations->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewReservations->setSelectionMode(QAbstractItemView::SingleSelection);

    loadUsers();
    loadRooms();
    loadReservations();

    connect(ui->tableViewReservations, &QTableView::clicked, this, &Reservations::on_tableViewReservations_clicked);
}




Reservations::~Reservations()
{
    delete ui;
}



void Reservations::loadUsers()
{
    ui->comboBoxUser->clear();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT user_id, username FROM users ORDER BY username")) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->comboBoxUser->addItem(name, id);
    }
}



void Reservations::loadRooms()
{
    ui->comboBoxRoom->clear();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT room_id, room_number FROM rooms ORDER BY room_number")) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString roomNum = query.value(1).toString();
        ui->comboBoxRoom->addItem(roomNum, id);
    }
}



void Reservations::loadReservations()
{
    m_model->select();
}




void Reservations::clearForm()
{
    ui->comboBoxUser->setCurrentIndex(-1);
    ui->comboBoxRoom->setCurrentIndex(-1);
    ui->dateEditCheckIn->setDate(QDate::currentDate());
    ui->dateEditCheckOut->setDate(QDate::currentDate().addDays(1));
    ui->spinBoxAmount->setValue(0.00);
    ui->comboBoxPayment->setCurrentIndex(-1);
    ui->comboBoxStatus->setCurrentIndex(-1);

    ui->tableViewReservations->clearSelection();
}



void Reservations::populateForm(int row)
{
    if(row < 0 || row >= m_model->rowCount())
        return;

    QSqlRecord record = m_model->record(row);

    int userId = record.value("user_id").toInt();
    int roomId = record.value("room_id").toInt();
    QDate checkIn = record.value("check_in_date").toDate();
    QDate checkOut = record.value("check_out_date").toDate();
    double amount = record.value("total_amount").toDouble();
    QString payment = record.value("payment_status").toString();
    QString status = record.value("reservation_status").toString();

    int userIndex = ui->comboBoxUser->findData(userId);
    ui->comboBoxUser->setCurrentIndex(userIndex);

    int roomIndex = ui->comboBoxRoom->findData(roomId);
    ui->comboBoxRoom->setCurrentIndex(roomIndex);

    ui->dateEditCheckIn->setDate(checkIn);
    ui->dateEditCheckOut->setDate(checkOut);
    ui->spinBoxAmount->setValue(amount);

    int paymentIndex = ui->comboBoxPayment->findText(payment);
    ui->comboBoxPayment->setCurrentIndex(paymentIndex);

    int statusIndex = ui->comboBoxStatus->findText(status);
    ui->comboBoxStatus->setCurrentIndex(statusIndex);
}




void Reservations::on_btnAdd_clicked()
{
    int userId = ui->comboBoxUser->currentData().toInt();
    int roomId = ui->comboBoxRoom->currentData().toInt();
    QDate checkIn = ui->dateEditCheckIn->date();
    QDate checkOut = ui->dateEditCheckOut->date();
    double amount = ui->spinBoxAmount->value();
    QString payment = ui->comboBoxPayment->currentText();
    QString status = ui->comboBoxStatus->currentText();

    if (checkOut <= checkIn) {
        QMessageBox::warning(this, "Input Error", "Check-out date must be after check-in date.");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO reservations (user_id, room_id, check_in_date, check_out_date, total_amount, payment_status, reservation_status) "
                  "VALUES (:user_id, :room_id, :check_in, :check_out, :amount, :payment, :status)");
    query.bindValue(":user_id", userId);
    query.bindValue(":room_id", roomId);
    query.bindValue(":check_in", checkIn);
    query.bindValue(":check_out", checkOut);
    query.bindValue(":amount", amount);
    query.bindValue(":payment", payment);
    query.bindValue(":status", status);

    if(!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
    } else {
        loadReservations();
        clearForm();
    }
}




void Reservations::on_btnUpdate_clicked()
{
    QModelIndexList selected = ui->tableViewReservations->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a reservation to update.");
        return;
    }

    int row = selected.first().row();
    int reservationId = m_model->record(row).value("reservation_id").toInt();

    int userId = ui->comboBoxUser->currentData().toInt();
    int roomId = ui->comboBoxRoom->currentData().toInt();
    QDate checkIn = ui->dateEditCheckIn->date();
    QDate checkOut = ui->dateEditCheckOut->date();
    double amount = ui->spinBoxAmount->value();
    QString payment = ui->comboBoxPayment->currentText();
    QString status = ui->comboBoxStatus->currentText();

    if (checkOut <= checkIn) {
        QMessageBox::warning(this, "Input Error", "Check-out date must be after check-in date.");
        return;
    }


    QSqlQuery query(m_db);
    query.prepare("UPDATE reservations SET user_id=:user_id, room_id=:room_id, check_in_date=:check_in, check_out_date=:check_out, "
                  "total_amount=:amount, payment_status=:payment, reservation_status=:status WHERE reservation_id=:id");
    query.bindValue(":user_id", userId);
    query.bindValue(":room_id", roomId);
    query.bindValue(":check_in", checkIn);
    query.bindValue(":check_out", checkOut);
    query.bindValue(":amount", amount);
    query.bindValue(":payment", payment);
    query.bindValue(":status", status);
    query.bindValue(":id", reservationId);

    if(!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
    } else {
        loadReservations();
        clearForm();
    }
}



void Reservations::on_btnDelete_clicked()
{
    QModelIndexList selected = ui->tableViewReservations->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a reservation to delete.");
        return;
    }

    int row = selected.first().row();
    int reservationId = m_model->record(row).value("reservation_id").toInt();

    if(QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this reservation?") == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM reservations WHERE reservation_id = :id");
        query.bindValue(":id", reservationId);

        if(!query.exec()) {
            QMessageBox::critical(this, "Database Error", query.lastError().text());
        } else {
            loadReservations();
            clearForm();
        }
    }
}



void Reservations::on_btnClear_clicked()
{
    clearForm();
}

void Reservations::on_btnRefresh_clicked()
{
    loadUsers();
    loadRooms();
    loadReservations();
    clearForm();
}

void Reservations::on_tableViewReservations_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    populateForm(index.row());
}

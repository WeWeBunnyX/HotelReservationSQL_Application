#include "reservations.h"
#include "ui_reservations.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QDate>
#include <QDebug>

Reservations::Reservations(QWidget *parent)
    : QWidget(parent), ui(new Ui::Reservations)
{
    ui->setupUi(this);

    m_db = QSqlDatabase::database("main_connection");

    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }

    m_model = new QSqlQueryModel(this);
    ui->tableViewReservations->setModel(m_model);
    ui->tableViewReservations->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewReservations->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->dateEditCheckIn->setDate(QDate::currentDate());
    ui->dateEditCheckOut->setDate(QDate::currentDate().addDays(1));
    ui->userNameDisplay->clear();
    ui->lineEditPreferred->clear();

    loadUsers();
    loadRooms();
    loadReservations();

    connect(ui->tableViewReservations, &QTableView::clicked, this, &Reservations::on_tableViewReservations_clicked);
    connect(ui->comboBoxUser, &QComboBox::currentIndexChanged, this, &Reservations::on_comboBoxUser_currentIndexChanged);
}

Reservations::~Reservations()
{
    delete ui;
}

void Reservations::loadUsers()
{
    ui->comboBoxUser->clear();
    ui->userNameDisplay->clear();
    ui->lineEditPreferred->clear();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT user_id FROM users WHERE role = 'Customer' ORDER BY user_id")) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value("user_id").toInt();
        ui->comboBoxUser->addItem(QString::number(id), id);
    }
}

void Reservations::loadRooms()
{
    ui->comboBoxRoom->clear();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT room_id, room_type FROM rooms ORDER BY room_type")) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value("room_id").toInt();
        QString roomType = query.value("room_type").toString();
        ui->comboBoxRoom->addItem(roomType, id);
    }
}

void Reservations::loadReservations()
{
    QString queryStr = R"(
        SELECT
            r.reservation_id,
            u.full_name AS user,
            rm.room_type AS room,
            r.check_in_date,
            r.check_out_date,
            r.total_amount,
            r.payment_status,
            r.reservation_status,
            r.user_id
        FROM reservations r
        JOIN users u ON r.user_id = u.user_id
        JOIN rooms rm ON r.room_id = rm.room_id
        ORDER BY r.check_in_date DESC
    )";

    m_model->setQuery(queryStr, m_db);

    m_model->setHeaderData(1, Qt::Horizontal, "User");
    m_model->setHeaderData(2, Qt::Horizontal, "Room");
    m_model->setHeaderData(3, Qt::Horizontal, "Check In Date");
    m_model->setHeaderData(4, Qt::Horizontal, "Check Out Date");
    m_model->setHeaderData(5, Qt::Horizontal, "Total Amount");
    m_model->setHeaderData(6, Qt::Horizontal, "Payment Status");
    m_model->setHeaderData(7, Qt::Horizontal, "Reservation Status");

    if (m_model->lastError().isValid()) {
        QMessageBox::critical(this, "Query Error", m_model->lastError().text());
    }

    ui->tableViewReservations->setColumnHidden(0, true);
    ui->tableViewReservations->setColumnHidden(8, true); // Hide user_id

    ui->tableViewReservations->setColumnWidth(1, 120); // User
    ui->tableViewReservations->setColumnWidth(2, 120); // Room
    ui->tableViewReservations->setColumnWidth(3, 110); // Check In Date
    ui->tableViewReservations->setColumnWidth(4, 130); // Check Out Date
    ui->tableViewReservations->setColumnWidth(5, 100); // Total Amount
    ui->tableViewReservations->setColumnWidth(6, 130); // Payment Status
    ui->tableViewReservations->setColumnWidth(7, 100); // Reservation Status
}

void Reservations::clearForm()
{
    ui->comboBoxUser->setCurrentIndex(-1);
    ui->userNameDisplay->clear();
    ui->lineEditPreferred->clear();
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
    if (row < 0 || row >= m_model->rowCount())
        return;

    QSqlRecord record = m_model->record(row);
    int userId = record.value("user_id").toInt();
    QString userName = record.value("user").toString();
    QString roomType = record.value("room").toString();
    QDate checkIn = record.value("check_in_date").toDate();
    QDate checkOut = record.value("check_out_date").toDate();
    double amount = record.value("total_amount").toDouble();
    QString payment = record.value("payment_status").toString();
    QString status = record.value("reservation_status").toString();

    int userIndex = ui->comboBoxUser->findData(userId);
    ui->comboBoxUser->setCurrentIndex(userIndex);
    ui->userNameDisplay->setText(userName);

    // Fetch preferred_room_type from customers table
    QSqlQuery query(m_db);
    query.prepare("SELECT preferred_room_type FROM customers WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);
    if (query.exec() && query.next()) {
        QString preferredRoomType = query.value("preferred_room_type").toString();
        ui->lineEditPreferred->setText(preferredRoomType);
        qDebug() << "Populated preferred_room_type for user_id" << userId << ":" << preferredRoomType;
    } else {
        ui->lineEditPreferred->clear();
        qDebug() << "No preferred_room_type found for user_id" << userId << "or query error:" << query.lastError().text();
    }

    int roomIndex = ui->comboBoxRoom->findText(roomType);
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
    if (ui->comboBoxUser->currentIndex() == -1 || ui->comboBoxRoom->currentIndex() == -1) {
        QMessageBox::warning(this, "Input Error", "Please select both a user and a room.");
        return;
    }

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

    if (!query.exec()) {
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

    QModelIndex index = selected.first();
    int row = index.row();
    int reservationId = m_model->record(row).value("reservation_id").toInt();

    if (ui->comboBoxUser->currentIndex() == -1 || ui->comboBoxRoom->currentIndex() == -1) {
        QMessageBox::warning(this, "Input Error", "Please select both a user and a room.");
        return;
    }

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

    if (!query.exec()) {
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

    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this reservation?") == QMessageBox::Yes)
    {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM reservations WHERE reservation_id = :id");
        query.bindValue(":id", reservationId);

        if (!query.exec()) {
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

void Reservations::on_comboBoxUser_currentIndexChanged(int index)
{
    ui->userNameDisplay->clear();
    ui->lineEditPreferred->clear();
    if (index >= 0) {
        int userId = ui->comboBoxUser->currentData().toInt();
        QSqlQuery query(m_db);
        query.prepare("SELECT u.full_name, c.preferred_room_type "
                      "FROM users u "
                      "LEFT JOIN customers c ON u.user_id = c.user_id "
                      "WHERE u.user_id = :user_id AND u.role = 'Customer'");
        query.bindValue(":user_id", userId);
        if (query.exec() && query.next()) {
            QString fullName = query.value("full_name").toString();
            QString preferredRoomType = query.value("preferred_room_type").toString();
            ui->userNameDisplay->setText(fullName);
            ui->lineEditPreferred->setText(preferredRoomType);
            qDebug() << "Fetched for user_id" << userId << ": full_name =" << fullName << ", preferred_room_type =" << preferredRoomType;
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to fetch user data: " + query.lastError().text());
            qDebug() << "Query error for user_id" << userId << ":" << query.lastError().text();
        }
    }
}

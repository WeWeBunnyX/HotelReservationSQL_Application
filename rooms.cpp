#include "rooms.h"
#include "ui_rooms.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDebug>

RoomsModule::RoomsModule(QWidget *parent)
    : QWidget(parent), ui(new Ui::RoomsModule)
{
    qDebug() << "RoomsModule created: " << this;
    ui->setupUi(this);

    m_db = QSqlDatabase::database("main_connection");
    if (!m_db.isValid() || !m_db.isOpen()) {
        qDebug() << "Database not connected or not open";
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }

    m_model = new QSqlQueryModel(this);
    ui->roomsTable->setModel(m_model);
    ui->roomsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->roomsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    loadRooms();

    connect(ui->addRoomButton, &QPushButton::clicked, this, &RoomsModule::on_addRoomButton_clicked);
    connect(ui->editRoomButton, &QPushButton::clicked, this, &RoomsModule::on_editRoomButton_clicked);
    connect(ui->deleteRoomButton, &QPushButton::clicked, this, &RoomsModule::on_deleteRoomButton_clicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &RoomsModule::on_searchLineEdit_textChanged);
    connect(ui->roomsTable, &QTableView::clicked, this, &RoomsModule::on_roomsTable_clicked);
}


RoomsModule::~RoomsModule()
{
    qDebug() << "RoomsModule destroyed: " << this;
    delete ui;
}



void RoomsModule::loadRooms(const QString &filter)
{
    QString queryStr = R"(
        SELECT
            r.room_id,
            r.room_type,
            r.price_per_night,
            r.status,
            r.description,
            COUNT(res.reservation_id) AS active_reservations
        FROM rooms r
        LEFT JOIN reservations res ON r.room_id = res.room_id
            AND res.reservation_status = 'Confirmed'
            AND CURRENT_DATE BETWEEN res.check_in_date AND res.check_out_date
        WHERE 1=1
    )";

    if (!filter.isEmpty()) {
        queryStr += " AND (LOWER(r.room_type) LIKE LOWER(:filter) OR LOWER(r.status) LIKE LOWER(:filter))";
    }

    queryStr += " GROUP BY r.room_id, r.room_type, r.price_per_night, r.status, r.description ORDER BY r.room_id";

    QSqlQuery query(m_db);
    query.prepare(queryStr);
    if (!filter.isEmpty()) {
        query.bindValue(":filter", "%" + filter + "%");
    }

    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError().text();
        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;
    }

    m_model->setQuery(query);
    if (m_model->lastError().isValid()) {
        qDebug() << "Model error:" << m_model->lastError().text();
        QMessageBox::critical(this, "Model Error", m_model->lastError().text());
        return;
    }

    m_model->setHeaderData(0, Qt::Horizontal, "Room ID");
    m_model->setHeaderData(1, Qt::Horizontal, "Room Type");
    m_model->setHeaderData(2, Qt::Horizontal, "Price Per Night");
    m_model->setHeaderData(3, Qt::Horizontal, "Status");
    m_model->setHeaderData(4, Qt::Horizontal, "Description");
    m_model->setHeaderData(5, Qt::Horizontal, "Active Reservations");

    ui->roomsTable->setColumnHidden(0, true);
    ui->roomsTable->setColumnWidth(1, 150);
    ui->roomsTable->setColumnWidth(2, 120);
    ui->roomsTable->setColumnWidth(3, 100);
    ui->roomsTable->setColumnWidth(4, 200);
    ui->roomsTable->setColumnWidth(5, 120);

    qDebug() << "Loaded" << m_model->rowCount() << "rows";
}


void RoomsModule::on_addRoomButton_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Add room button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Add room button clicked";
    ui->addRoomButton->setEnabled(false);
    ui->addRoomButton->clearFocus();
    disconnect(ui->addRoomButton, &QPushButton::clicked, this, &RoomsModule::on_addRoomButton_clicked);

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Add Room");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QFormLayout form(&dialog);
    QLineEdit roomTypeEdit(&dialog);
    QDoubleSpinBox priceSpin(&dialog);
    priceSpin.setDecimals(2);
    priceSpin.setMaximum(10000.00);
    priceSpin.setMinimum(0.00);
    QComboBox statusCombo(&dialog);
    statusCombo.addItems({"Available", "Booked"});
    QLineEdit descriptionEdit(&dialog);

    form.addRow("Room Type:", &roomTypeEdit);
    form.addRow("Price Per Night:", &priceSpin);
    form.addRow("Status:", &statusCombo);
    form.addRow("Description:", &descriptionEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);

    disconnect(&buttonBox, nullptr, nullptr, nullptr);
    connect(&buttonBox, &QDialogButtonBox::accepted, [&dialog]() {
        qDebug() << "Dialog accepted";
        dialog.accept();
    });
    connect(&buttonBox, &QDialogButtonBox::rejected, [&dialog]() {
        qDebug() << "Dialog rejected";
        dialog.reject();
    });

    dialog.setFocus();
    int result = dialog.exec();

    qDebug() << "Dialog closed with result:" << result;
    dialog.clearFocus();
    ui->addRoomButton->setEnabled(true);
    connect(ui->addRoomButton, &QPushButton::clicked, this, &RoomsModule::on_addRoomButton_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        QString roomType = roomTypeEdit.text().trimmed();
        double price = priceSpin.value();
        QString status = statusCombo.currentText();
        QString description = descriptionEdit.text().trimmed();

        if (roomType.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Room Type is required.");
            return;
        }

        m_db.transaction();
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO rooms (room_type, price_per_night, status, description) "
                      "VALUES (:room_type, :price, :status, :description)");
        query.bindValue(":room_type", roomType);
        query.bindValue(":price", price);
        query.bindValue(":status", status);
        query.bindValue(":description", description.isEmpty() ? QVariant() : description);

        if (query.exec()) {
            m_db.commit();
            qDebug() << "Room added:" << roomType << price << status << description;
            loadRooms(ui->searchLineEdit->text());
        } else {
            m_db.rollback();
            qDebug() << "Insert error:" << query.lastError().text();
            QMessageBox::critical(this, "Database Error", query.lastError().text());
        }
    }
}



void RoomsModule::on_editRoomButton_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Edit room button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Edit room button clicked";
    ui->editRoomButton->setEnabled(false);
    ui->editRoomButton->clearFocus();
    disconnect(ui->editRoomButton, &QPushButton::clicked, this, &RoomsModule::on_editRoomButton_clicked);

    QModelIndexList selected = ui->roomsTable->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a room to edit.");
        ui->editRoomButton->setEnabled(true);
        connect(ui->editRoomButton, &QPushButton::clicked, this, &RoomsModule::on_editRoomButton_clicked);
        isProcessing = false;
        return;
    }

    int row = selected.first().row();
    int roomId = m_model->data(m_model->index(row, 0)).toInt();
    QString roomType = m_model->data(m_model->index(row, 1)).toString();
    double price = m_model->data(m_model->index(row, 2)).toDouble();
    QString status = m_model->data(m_model->index(row, 3)).toString();
    QString description = m_model->data(m_model->index(row, 4)).toString();

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Edit Room");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QFormLayout form(&dialog);
    QLineEdit roomTypeEdit(roomType, &dialog);
    QDoubleSpinBox priceSpin(&dialog);
    priceSpin.setDecimals(2);
    priceSpin.setMaximum(10000.00);
    priceSpin.setMinimum(0.00);
    priceSpin.setValue(price);
    QComboBox statusCombo(&dialog);
    statusCombo.addItems({"Available", "Booked"});
    statusCombo.setCurrentText(status);
    QLineEdit descriptionEdit(description, &dialog);

    form.addRow("Room Type:", &roomTypeEdit);
    form.addRow("Price Per Night:", &priceSpin);
    form.addRow("Status:", &statusCombo);
    form.addRow("Description:", &descriptionEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);

    disconnect(&buttonBox, nullptr, nullptr, nullptr);
    connect(&buttonBox, &QDialogButtonBox::accepted, [&dialog]() {
        qDebug() << "Dialog accepted";
        dialog.accept();
    });
    connect(&buttonBox, &QDialogButtonBox::rejected, [&dialog]() {
        qDebug() << "Dialog rejected";
        dialog.reject();
    });

    dialog.setFocus();
    int result = dialog.exec();

    qDebug() << "Dialog closed with result:" << result;
    dialog.clearFocus();
    ui->editRoomButton->setEnabled(true);
    connect(ui->editRoomButton, &QPushButton::clicked, this, &RoomsModule::on_editRoomButton_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        QString newRoomType = roomTypeEdit.text().trimmed();
        double newPrice = priceSpin.value();
        QString newStatus = statusCombo.currentText();
        QString newDescription = descriptionEdit.text().trimmed();

        if (newRoomType.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Room Type is required.");
            return;
        }

        m_db.transaction();
        QSqlQuery query(m_db);
        query.prepare("UPDATE rooms SET room_type = :room_type, price_per_night = :price, "
                      "status = :status, description = :description WHERE room_id = :room_id");
        query.bindValue(":room_type", newRoomType);
        query.bindValue(":price", newPrice);
        query.bindValue(":status", newStatus);
        query.bindValue(":description", newDescription.isEmpty() ? QVariant() : newDescription);
        query.bindValue(":room_id", roomId);

        if (query.exec()) {
            m_db.commit();
            qDebug() << "Room updated:" << newRoomType << newPrice << newStatus << newDescription;
            loadRooms(ui->searchLineEdit->text());
        } else {
            m_db.rollback();
            qDebug() << "Update error:" << query.lastError().text();
            QMessageBox::critical(this, "Database Error", query.lastError().text());
        }
    }
}



void RoomsModule::on_deleteRoomButton_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Delete room button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Delete room button clicked";
    ui->deleteRoomButton->setEnabled(false);
    ui->deleteRoomButton->clearFocus();
    disconnect(ui->deleteRoomButton, &QPushButton::clicked, this, &RoomsModule::on_deleteRoomButton_clicked);

    QModelIndexList selected = ui->roomsTable->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a room to delete.");
        ui->deleteRoomButton->setEnabled(true);
        connect(ui->deleteRoomButton, &QPushButton::clicked, this, &RoomsModule::on_deleteRoomButton_clicked);
        isProcessing = false;
        return;
    }

    int row = selected.first().row();
    int roomId = m_model->data(m_model->index(row, 0)).toInt();

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Delete Room");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout layout(&dialog);
    QLabel label("Are you sure you want to delete this room? This will also delete related reservations.", &dialog);
    layout.addWidget(&label);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(&buttonBox);

    disconnect(&buttonBox, nullptr, nullptr, nullptr);
    connect(&buttonBox, &QDialogButtonBox::accepted, [&dialog]() {
        qDebug() << "Dialog accepted";
        dialog.accept();
    });
    connect(&buttonBox, &QDialogButtonBox::rejected, [&dialog]() {
        qDebug() << "Dialog rejected";
        dialog.reject();
    });

    dialog.setFocus();
    int result = dialog.exec();

    qDebug() << "Dialog closed with result:" << result;
    dialog.clearFocus();
    ui->deleteRoomButton->setEnabled(true);
    connect(ui->deleteRoomButton, &QPushButton::clicked, this, &RoomsModule::on_deleteRoomButton_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        m_db.transaction();
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM rooms WHERE room_id = :room_id");
        query.bindValue(":room_id", roomId);

        if (query.exec()) {
            m_db.commit();
            qDebug() << "Room deleted: ID" << roomId;
            loadRooms(ui->searchLineEdit->text());
        } else {
            m_db.rollback();
            qDebug() << "Delete error:" << query.lastError().text();
            QMessageBox::critical(this, "Database Error", query.lastError().text());
        }
    }
}


void RoomsModule::on_searchLineEdit_textChanged(const QString &text)
{
    qDebug() << "Search text changed:" << text;
    loadRooms(text);
}


void RoomsModule::on_roomsTable_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qDebug() << "Invalid table index clicked";
        ui->roomsTable->clearSelection();
    }
}

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
#include <QPushButton>
#include <QVBoxLayout>

RoomsModule::RoomsModule(QWidget *parent)
    : QWidget(parent), ui(new Ui::RoomsModule)
{
    ui->setupUi(this);

    m_db = QSqlDatabase::database("main_connection");
    if (!m_db.isValid() || !m_db.isOpen()) {
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

    m_model->setQuery(query);

    m_model->setHeaderData(0, Qt::Horizontal, "Room ID");
    m_model->setHeaderData(1, Qt::Horizontal, "Room Type");
    m_model->setHeaderData(2, Qt::Horizontal, "Price Per Night");
    m_model->setHeaderData(3, Qt::Horizontal, "Status");
    m_model->setHeaderData(4, Qt::Horizontal, "Description");
    m_model->setHeaderData(5, Qt::Horizontal, "Active Reservations");

    if (m_model->lastError().isValid()) {
        QMessageBox::critical(this, "Query Error", m_model->lastError().text());
        return;
    }

    ui->roomsTable->setColumnHidden(0, true);
    ui->roomsTable->setColumnWidth(1, 150); // Room Type
    ui->roomsTable->setColumnWidth(2, 120); // Price Per Night
    ui->roomsTable->setColumnWidth(3, 100); // Status
    ui->roomsTable->setColumnWidth(4, 200); // Description
    ui->roomsTable->setColumnWidth(5, 120); // Active Reservations
}

void RoomsModule::on_addRoomButton_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add Room");

    QFormLayout *form = new QFormLayout;
    QLineEdit *roomTypeEdit = new QLineEdit(&dialog);
    QDoubleSpinBox *priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setDecimals(2);
    priceSpin->setMaximum(10000.00);
    priceSpin->setMinimum(0.00);
    QComboBox *statusCombo = new QComboBox(&dialog);
    statusCombo->addItems({"Available", "Booked"});
    QLineEdit *descriptionEdit = new QLineEdit(&dialog);

    form->addRow("Room Type:", roomTypeEdit);
    form->addRow("Price Per Night:", priceSpin);
    form->addRow("Status:", statusCombo);
    form->addRow("Description:", descriptionEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *saveButton = new QPushButton("Save", &dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttonLayout);
    dialog.setLayout(mainLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, [&]() {
        QString roomType = roomTypeEdit->text().trimmed();
        double price = priceSpin->value();
        QString status = statusCombo->currentText();
        QString description = descriptionEdit->text().trimmed();

        if (roomType.isEmpty()) {
            QMessageBox::warning(&dialog, "Input Error", "Room Type is required.");
            return;
        }

        QSqlQuery query(m_db);
        query.prepare("INSERT INTO rooms (room_type, price_per_night, status, description) "
                      "VALUES (:room_type, :price, :status, :description)");
        query.bindValue(":room_type", roomType);
        query.bindValue(":price", price);
        query.bindValue(":status", status);
        query.bindValue(":description", description.isEmpty() ? QVariant() : description);

        if (!query.exec()) {
            QMessageBox::critical(&dialog, "Database Error", query.lastError().text());
        } else {
            dialog.accept();
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        loadRooms(ui->searchLineEdit->text());
    }
}

void RoomsModule::on_editRoomButton_clicked()
{
    QModelIndexList selected = ui->roomsTable->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a room to edit.");
        return;
    }

    int row = selected.first().row();
    int roomId = m_model->data(m_model->index(row, 0)).toInt();
    QString roomType = m_model->data(m_model->index(row, 1)).toString();
    double price = m_model->data(m_model->index(row, 2)).toDouble();
    QString status = m_model->data(m_model->index(row, 3)).toString();
    QString description = m_model->data(m_model->index(row, 4)).toString();

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Room");

    QFormLayout *form = new QFormLayout;
    QLineEdit *roomTypeEdit = new QLineEdit(roomType, &dialog);
    QDoubleSpinBox *priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setDecimals(2);
    priceSpin->setMaximum(10000.00);
    priceSpin->setMinimum(0.00);
    priceSpin->setValue(price);
    QComboBox *statusCombo = new QComboBox(&dialog);
    statusCombo->addItems({"Available", "Booked"});
    statusCombo->setCurrentText(status);
    QLineEdit *descriptionEdit = new QLineEdit(description, &dialog);

    form->addRow("Room Type:", roomTypeEdit);
    form->addRow("Price Per Night:", priceSpin);
    form->addRow("Status:", statusCombo);
    form->addRow("Description:", descriptionEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *saveButton = new QPushButton("Save", &dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttonLayout);
    dialog.setLayout(mainLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, [&]() {
        QString newRoomType = roomTypeEdit->text().trimmed();
        double newPrice = priceSpin->value();
        QString newStatus = statusCombo->currentText();
        QString newDescription = descriptionEdit->text().trimmed();

        if (newRoomType.isEmpty()) {
            QMessageBox::warning(&dialog, "Input Error", "Room Type is required.");
            return;
        }

        QSqlQuery query(m_db);
        query.prepare("UPDATE rooms SET room_type = :room_type, price_per_night = :price, "
                      "status = :status, description = :description WHERE room_id = :room_id");
        query.bindValue(":room_type", newRoomType);
        query.bindValue(":price", newPrice);
        query.bindValue(":status", newStatus);
        query.bindValue(":description", newDescription.isEmpty() ? QVariant() : newDescription);
        query.bindValue(":room_id", roomId);

        if (!query.exec()) {
            QMessageBox::critical(&dialog, "Database Error", query.lastError().text());
        } else {
            dialog.accept();
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        loadRooms(ui->searchLineEdit->text());
    }
}

void RoomsModule::on_deleteRoomButton_clicked()
{
    QModelIndexList selected = ui->roomsTable->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a room to delete.");
        return;
    }

    int row = selected.first().row();
    int roomId = m_model->data(m_model->index(row, 0)).toInt();

    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this room? "
                              "This will also delete related reservations.") == QMessageBox::Yes)
    {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM rooms WHERE room_id = :room_id");
        query.bindValue(":room_id", roomId);

        if (!query.exec()) {
            QMessageBox::critical(this, "Database Error", query.lastError().text());
        } else {
            loadRooms(ui->searchLineEdit->text());
        }
    }
}

void RoomsModule::on_searchLineEdit_textChanged(const QString &text)
{
    loadRooms(text);
}

void RoomsModule::on_roomsTable_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        ui->roomsTable->clearSelection();
    }
}

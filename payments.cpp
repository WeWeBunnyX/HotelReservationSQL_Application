#include "payments.h"
#include "ui_payments.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QRandomGenerator>
#include <utility>

PaymentsModule::PaymentsModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaymentsModule)
{
    qDebug() << "PaymentsModule created: " << this;
    ui->setupUi(this);

    m_db = QSqlDatabase::database("main_connection");
    if (!m_db.isValid() || !m_db.isOpen()) {
        qDebug() << "Database not connected or not open";
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }

    m_model = new QSqlQueryModel(this);
    ui->tablePayments->setModel(m_model);
    ui->tablePayments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablePayments->setSelectionMode(QAbstractItemView::SingleSelection);

    populateUserComboBox();
    populateReservationComboBox(); // Load all confirmed reservations initially
    loadPayments();
    clearForm();
    setupConnections();
}

PaymentsModule::~PaymentsModule()
{
    qDebug() << "PaymentsModule destroyed: " << this;
    delete ui;
}

void PaymentsModule::setupConnections()
{
    if (m_connectionsSetup) {
        qDebug() << "Connections already setup for PaymentsModule:" << this;
        return;
    }
    connect(ui->buttonAdd, &QPushButton::clicked, this, &PaymentsModule::on_buttonAdd_clicked);
    connect(ui->buttonUpdate, &QPushButton::clicked, this, &PaymentsModule::on_buttonUpdate_clicked);
    connect(ui->buttonDelete, &QPushButton::clicked, this, &PaymentsModule::on_buttonDelete_clicked);
    connect(ui->buttonClear, &QPushButton::clicked, this, &PaymentsModule::on_buttonClear_clicked);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &PaymentsModule::on_lineEditSearch_textChanged);
    connect(ui->tablePayments, &QTableView::clicked, this, &PaymentsModule::on_tablePayments_clicked);
    connect(ui->comboBoxUserID, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PaymentsModule::on_comboBoxUserID_currentIndexChanged);
    m_connectionsSetup = true;
    qDebug() << "Connections setup for PaymentsModule:" << this;
}

void PaymentsModule::loadPayments(const QString &filter)
{
    if (!m_db.isOpen()) {
        m_db.open();
        if (!m_db.isOpen()) {
            qDebug() << "Failed to reopen database in loadPayments:" << m_db.lastError().text();
            QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
            return;
        }
    }

    QString queryStr = R"(
        SELECT
            r.reservation_id,
            r.user_id,
            u.full_name AS user_name,
            r.total_amount,
            r.payment_status,
            r.payment_method,
            r.payment_date,
            r.transaction_id,
            r.payment_notes
        FROM reservations r
        JOIN users u ON r.user_id = u.user_id
        WHERE 1=1
    )";

    if (!filter.isEmpty()) {
        queryStr += " AND CAST(r.reservation_id AS TEXT) LIKE :filter";
    }

    queryStr += " ORDER BY r.reservation_id DESC";

    QSqlQuery query(m_db);
    query.prepare(queryStr);
    if (!filter.isEmpty()) {
        query.bindValue(":filter", "%" + filter + "%");
    }

    if (!query.exec()) {
        qDebug() << "Query error in loadPayments:" << query.lastError().text();
        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;
    }

    m_model->setQuery(std::move(query));
    if (m_model->lastError().isValid()) {
        qDebug() << "Model error:" << m_model->lastError().text();
        QMessageBox::critical(this, "Model Error", m_model->lastError().text());
        return;
    }

    m_model->setHeaderData(0, Qt::Horizontal, "Reservation ID");
    m_model->setHeaderData(1, Qt::Horizontal, "User ID");
    m_model->setHeaderData(2, Qt::Horizontal, "User Name");
    m_model->setHeaderData(3, Qt::Horizontal, "Amount");
    m_model->setHeaderData(4, Qt::Horizontal, "Payment Status");
    m_model->setHeaderData(5, Qt::Horizontal, "Payment Method");
    m_model->setHeaderData(6, Qt::Horizontal, "Payment Date");
    m_model->setHeaderData(7, Qt::Horizontal, "Transaction ID");
    m_model->setHeaderData(8, Qt::Horizontal, "Notes");

    ui->tablePayments->setColumnHidden(0, true);
    ui->tablePayments->setColumnHidden(1, true);
    ui->tablePayments->setColumnWidth(2, 150);
    ui->tablePayments->setColumnWidth(3, 100);
    ui->tablePayments->setColumnWidth(4, 100);
    ui->tablePayments->setColumnWidth(5, 120);
    ui->tablePayments->setColumnWidth(6, 120);
    ui->tablePayments->setColumnWidth(7, 100);
    ui->tablePayments->setColumnWidth(8, 200);

    qDebug() << "Loaded" << m_model->rowCount() << "payments";
}

void PaymentsModule::populateUserComboBox()
{
    if (!m_db.isOpen()) {
        m_db.open();
        if (!m_db.isOpen()) {
            qDebug() << "Failed to reopen database in populateUserComboBox:" << m_db.lastError().text();
            QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
            return;
        }
    }

    ui->comboBoxUserID->clear();
    QSqlQuery query(m_db);
    query.prepare("SELECT user_id, full_name FROM users WHERE role = 'Customer' ORDER BY user_id");
    if (!query.exec()) {
        qDebug() << "User query error:" << query.lastError().text();
        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        ui->comboBoxUserID->addItem(query.value(0).toString(), query.value(1).toString());
    }
    qDebug() << "Populated" << ui->comboBoxUserID->count() << "user IDs";
}

void PaymentsModule::populateReservationComboBox(int userId)
{
    if (!m_db.isOpen()) {
        m_db.open();
        if (!m_db.isOpen()) {
            qDebug() << "Failed to reopen database in populateReservationComboBox:" << m_db.lastError().text();
            QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
            return;
        }
    }

    ui->comboBoxReservationID->clear();
    QSqlQuery query(m_db);
    if (userId >= 0) {
        query.prepare(R"(
            SELECT r.reservation_id, u.user_id, u.full_name
            FROM reservations r
            JOIN users u ON r.user_id = u.user_id
            WHERE r.reservation_status = 'Confirmed' AND r.user_id = :user_id
            ORDER BY r.reservation_id
        )");
        query.bindValue(":user_id", userId);
    } else {
        query.prepare(R"(
            SELECT r.reservation_id, u.user_id, u.full_name
            FROM reservations r
            JOIN users u ON r.user_id = u.user_id
            WHERE r.reservation_status = 'Confirmed'
            ORDER BY r.reservation_id
        )");
    }

    if (!query.exec()) {
        qDebug() << "Reservation query error:" << query.lastError().text();
        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        QString reservationId = query.value(0).toString();
        QString fullName = query.value(2).toString();
        ui->comboBoxReservationID->addItem(reservationId, QVariantList({query.value(1).toInt(), fullName}));
    }
    qDebug() << "Populated" << ui->comboBoxReservationID->count() << "reservation IDs";
}

void PaymentsModule::clearForm()
{
    ui->comboBoxUserID->setCurrentIndex(-1);
    ui->lineEditUserName->clear();
    ui->lineEditReservationID->clear();
    ui->comboBoxReservationID->setCurrentIndex(-1);
    ui->lineEditAmount->clear();
    ui->comboBoxPaymentStatus->setCurrentIndex(-1);
    ui->comboBoxPaymentMethod->setCurrentIndex(-1);
    ui->dateEditPaymentDate->setDate(QDate::currentDate());
    ui->lineEditTransactionID->clear();
    ui->textEditPaymentNotes->clear();
    qDebug() << "Form cleared";
}

bool PaymentsModule::validateForm(int &userId, int &reservationId, double &amount, QString &paymentStatus, QString &method, QDate &date, QString &transactionId, QString &notes)
{
    bool ok;
    userId = ui->comboBoxUserID->currentText().toInt(&ok);
    if (!ok || userId <= 0) {
        QMessageBox::warning(this, "Input Error", "Invalid User ID.");
        return false;
    }

    reservationId = ui->comboBoxReservationID->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Input Error", "Invalid Reservation ID.");
        return false;
    }

    amount = ui->lineEditAmount->text().toDouble(&ok);
    if (!ok || amount < 0) {
        QMessageBox::warning(this, "Input Error", "Amount must be a non-negative number.");
        return false;
    }

    paymentStatus = ui->comboBoxPaymentStatus->currentText();
    if (paymentStatus.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Payment Status is required.");
        return false;
    }

    method = ui->comboBoxPaymentMethod->currentText();
    if (method.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Payment Method is required.");
        return false;
    }

    date = ui->dateEditPaymentDate->date();
    if (!date.isValid()) {
        QMessageBox::warning(this, "Input Error", "Invalid Payment Date.");
        return false;
    }

    transactionId = ui->lineEditTransactionID->text().trimmed();
    if (!transactionId.isEmpty()) {
        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT reservation_id FROM reservations WHERE transaction_id = :transaction_id AND reservation_id != :reservation_id");
        checkQuery.bindValue(":transaction_id", transactionId);
        checkQuery.bindValue(":reservation_id", reservationId);
        if (checkQuery.exec() && checkQuery.next()) {
            QMessageBox::warning(this, "Input Error", "Transaction ID already exists.");
            return false;
        }
    }

    notes = ui->textEditPaymentNotes->toPlainText().trimmed();
    return true;
}

QString PaymentsModule::generateUniqueTransactionId()
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString transactionId;
    bool unique = false;
    int attempts = 0;
    const int maxAttempts = 100;

    while (!unique && attempts < maxAttempts) {
        transactionId.clear();
        for (int i = 0; i < 4; ++i) {
            transactionId.append(chars[QRandomGenerator::global()->bounded(chars.length())]);
        }

        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT transaction_id FROM reservations WHERE transaction_id = :transaction_id");
        checkQuery.bindValue(":transaction_id", transactionId);
        if (!checkQuery.exec()) {
            qDebug() << "Transaction ID check error:" << checkQuery.lastError().text();
            return QString();
        }
        unique = !checkQuery.next();
        attempts++;
    }

    if (!unique) {
        QMessageBox::warning(this, "Error", "Could not generate unique Transaction ID.");
        return QString();
    }
    return transactionId;
}

void PaymentsModule::on_comboBoxUserID_currentIndexChanged(int index)
{
    if (index < 0) {
        ui->lineEditUserName->clear();
        populateReservationComboBox();
        return;
    }
    QString userName = ui->comboBoxUserID->itemData(index, Qt::UserRole).toString();
    ui->lineEditUserName->setText(userName);
    int userId = ui->comboBoxUserID->currentText().toInt();
    populateReservationComboBox(userId);
    if (ui->comboBoxReservationID->count() > 0) {
        ui->lineEditReservationID->setText(ui->comboBoxReservationID->currentText());
    } else {
        ui->lineEditReservationID->clear();
    }
}

void PaymentsModule::on_buttonAdd_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Add button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Add button clicked";
    ui->buttonAdd->setEnabled(false);
    ui->buttonAdd->clearFocus();
    disconnect(ui->buttonAdd, &QPushButton::clicked, this, &PaymentsModule::on_buttonAdd_clicked);

    int userId, reservationId;
    double amount;
    QString paymentStatus, method, transactionId, notes;
    QDate date;

    // Autogenerate transaction ID
    transactionId = generateUniqueTransactionId();
    if (transactionId.isEmpty()) {
        ui->buttonAdd->setEnabled(true);
        connect(ui->buttonAdd, &QPushButton::clicked, this, &PaymentsModule::on_buttonAdd_clicked);
        isProcessing = false;
        return;
    }
    ui->lineEditTransactionID->setText(transactionId);

    if (!validateForm(userId, reservationId, amount, paymentStatus, method, date, transactionId, notes)) {
        ui->buttonAdd->setEnabled(true);
        connect(ui->buttonAdd, &QPushButton::clicked, this, &PaymentsModule::on_buttonAdd_clicked);
        isProcessing = false;
        return;
    }

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Confirm Add Payment");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout layout(&dialog);
    QLabel label(QString("Add payment of $%1 for User %2, Reservation %3 (%4) via %5 on %6?\nTransaction ID: %7%8")
                 .arg(amount, 0, 'f', 2)
                 .arg(userId)
                 .arg(reservationId)
                 .arg(paymentStatus)
                 .arg(method)
                 .arg(date.toString("yyyy-MM-dd"))
                 .arg(transactionId)
                 .arg(notes.isEmpty() ? "" : "\nNotes: " + notes), &dialog);
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
    ui->buttonAdd->setEnabled(true);
    connect(ui->buttonAdd, &QPushButton::clicked, this, &PaymentsModule::on_buttonAdd_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        if (!m_db.isOpen()) {
            m_db.open();
            if (!m_db.isOpen()) {
                qDebug() << "Failed to reopen database in on_buttonAdd_clicked:" << m_db.lastError().text();
                QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
                return;
            }
        }

        m_db.transaction();
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(R"(
            INSERT INTO reservations (
                reservation_id, user_id, room_id, check_in_date, check_out_date,
                total_amount, payment_status, reservation_status, payment_method,
                payment_date, transaction_id, payment_notes
            ) VALUES (
                :reservation_id, :user_id, :room_id, :check_in_date, :check_out_date,
                :total_amount, :payment_status, :reservation_status, :payment_method,
                :payment_date, :transaction_id, :payment_notes
            )
        )");
        insertQuery.bindValue(":reservation_id", reservationId);
        insertQuery.bindValue(":user_id", userId);
        insertQuery.bindValue(":room_id", 1); // Placeholder: Adjust based on your logic
        insertQuery.bindValue(":check_in_date", QDate::currentDate());
        insertQuery.bindValue(":check_out_date", QDate::currentDate().addDays(1));
        insertQuery.bindValue(":total_amount", amount);
        insertQuery.bindValue(":payment_status", paymentStatus);
        insertQuery.bindValue(":reservation_status", "Confirmed");
        insertQuery.bindValue(":payment_method", method);
        insertQuery.bindValue(":payment_date", date);
        insertQuery.bindValue(":transaction_id", transactionId.isEmpty() ? QVariant() : transactionId);
        insertQuery.bindValue(":payment_notes", notes.isEmpty() ? QVariant() : notes);

        if (!insertQuery.exec()) {
            m_db.rollback();
            qDebug() << "Insert error:" << insertQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", insertQuery.lastError().text());
            return;
        }

        if (!m_db.commit()) {
            m_db.rollback();
            qDebug() << "Commit error:" << m_db.lastError().text();
            QMessageBox::critical(this, "Database Error", m_db.lastError().text());
            return;
        }

        qDebug() << "Payment added: User" << userId << "Reservation" << reservationId << amount << paymentStatus << method << date;
        loadPayments(ui->lineEditSearch->text());
        clearForm();
    }
}

void PaymentsModule::on_buttonUpdate_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Update button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Update button clicked";
    ui->buttonUpdate->setEnabled(false);
    ui->buttonUpdate->clearFocus();
    disconnect(ui->buttonUpdate, &QPushButton::clicked, this, &PaymentsModule::on_buttonUpdate_clicked);

    QModelIndexList selected = ui->tablePayments->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a payment to update.");
        ui->buttonUpdate->setEnabled(true);
        connect(ui->buttonUpdate, &QPushButton::clicked, this, &PaymentsModule::on_buttonUpdate_clicked);
        isProcessing = false;
        return;
    }

    int row = selected.first().row();
    int reservationId = m_model->data(m_model->index(row, 0)).toInt();

    int userId;
    double amount;
    QString paymentStatus, method, transactionId, notes;
    QDate date;

    if (!validateForm(userId, reservationId, amount, paymentStatus, method, date, transactionId, notes)) {
        ui->buttonUpdate->setEnabled(true);
        connect(ui->buttonUpdate, &QPushButton::clicked, this, &PaymentsModule::on_buttonUpdate_clicked);
        isProcessing = false;
        return;
    }

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Confirm Update Payment");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout layout(&dialog);
    QLabel label(QString("Update payment for User %1, Reservation %2 to $%3 (%4) via %5 on %6?\nTransaction ID: %7%8")
                 .arg(userId)
                 .arg(reservationId)
                 .arg(amount, 0, 'f', 2)
                 .arg(paymentStatus)
                 .arg(method)
                 .arg(date.toString("yyyy-MM-dd"))
                 .arg(transactionId)
                 .arg(notes.isEmpty() ? "" : "\nNotes: " + notes), &dialog);
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
    ui->buttonUpdate->setEnabled(true);
    connect(ui->buttonUpdate, &QPushButton::clicked, this, &PaymentsModule::on_buttonUpdate_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        if (!m_db.isOpen()) {
            m_db.open();
            if (!m_db.isOpen()) {
                qDebug() << "Failed to reopen database in on_buttonUpdate_clicked:" << m_db.lastError().text();
                QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
                return;
            }
        }

        m_db.transaction();
        QSqlQuery updateQuery(m_db);
        updateQuery.prepare(R"(
            UPDATE reservations
            SET user_id = :user_id,
                total_amount = :total_amount,
                payment_status = :payment_status,
                payment_method = :payment_method,
                payment_date = :payment_date,
                transaction_id = :transaction_id,
                payment_notes = :payment_notes
            WHERE reservation_id = :reservation_id
        )");
        updateQuery.bindValue(":user_id", userId);
        updateQuery.bindValue(":total_amount", amount);
        updateQuery.bindValue(":payment_status", paymentStatus);
        updateQuery.bindValue(":payment_method", method);
        updateQuery.bindValue(":payment_date", date);
        updateQuery.bindValue(":transaction_id", transactionId.isEmpty() ? QVariant() : transactionId);
        updateQuery.bindValue(":payment_notes", notes.isEmpty() ? QVariant() : notes);
        updateQuery.bindValue(":reservation_id", reservationId);

        if (!updateQuery.exec()) {
            m_db.rollback();
            qDebug() << "Update error:" << updateQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", updateQuery.lastError().text());
            return;
        }

        if (!m_db.commit()) {
            m_db.rollback();
            qDebug() << "Commit error:" << m_db.lastError().text();
            QMessageBox::critical(this, "Database Error", m_db.lastError().text());
            return;
        }

        qDebug() << "Payment updated: User" << userId << " Reservation" << reservationId << amount;
        loadPayments(ui->lineEditSearch->text());
        clearForm();
    }
}

void PaymentsModule::on_buttonDelete_clicked()
{
    static bool isProcessing = false;
    if (isProcessing) {
        qDebug() << "Delete button ignored: already processing";
        return;
    }
    isProcessing = true;
    qDebug() << "Delete button clicked";
    ui->buttonDelete->setEnabled(false);
    ui->buttonDelete->clearFocus();
    disconnect(ui->buttonDelete, &QPushButton::clicked, this, &PaymentsModule::on_buttonDelete_clicked);

    QModelIndexList selected = ui->tablePayments->selectionModel()->selectedRows();
    if (selected.empty()) {
        QMessageBox::warning(this, "Selection Error", "Select a payment to delete.");
        ui->buttonDelete->setEnabled(true);
        connect(ui->buttonDelete, &QPushButton::clicked, this, &PaymentsModule::on_buttonDelete_clicked);
        isProcessing = false;
        return;
    }

    int row = selected.first().row();
    int reservationId = m_model->data(m_model->index(row, 0)).toInt();

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Delete Payment");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout layout(&dialog);
    QLabel label(QString("Are you sure you want to delete payment details for Reservation %1?").arg(reservationId), &dialog);
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
    ui->buttonDelete->setEnabled(true);
    connect(ui->buttonDelete, &QPushButton::clicked, this, &PaymentsModule::on_buttonDelete_clicked);
    isProcessing = false;

    if (result == QDialog::Accepted) {
        if (!m_db.isOpen()) {
            m_db.open();
            if (!m_db.isOpen()) {
                qDebug() << "Failed to reopen database in on_buttonDelete_clicked:" << m_db.lastError().text();
                QMessageBox::critical(this, "DB Error", "Cannot reopen database connection.");
                return;
            }
        }

        m_db.transaction();
        QSqlQuery deleteQuery(m_db);
        deleteQuery.prepare(R"(
            UPDATE reservations
            SET total_amount = 0,
                payment_status = 'Pending',
                payment_method = NULL,
                payment_date = NULL,
                transaction_id = NULL,
                payment_notes = NULL
            WHERE reservation_id = :reservation_id
        )");
        deleteQuery.bindValue(":reservation_id", reservationId);

        if (!deleteQuery.exec()) {
            m_db.rollback();
            qDebug() << "Delete error:" << deleteQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", deleteQuery.lastError().text());
            return;
        }

        if (!m_db.commit()) {
            m_db.rollback();
            qDebug() << "Commit error:" << m_db.lastError().text();
            QMessageBox::critical(this, "Database Error", m_db.lastError().text());
            return;
        }

        qDebug() << "Payment deleted: Reservation" << reservationId;
        loadPayments(ui->lineEditSearch->text());
        clearForm();
    }
}

void PaymentsModule::on_buttonClear_clicked()
{
    qDebug() << "Clear button clicked";
    clearForm();
}

void PaymentsModule::on_lineEditSearch_textChanged(const QString &text)
{
    qDebug() << "Search text changed:" << text;
    loadPayments(text);
}

void PaymentsModule::on_tablePayments_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qDebug() << "Invalid table index clicked";
        ui->tablePayments->clearSelection();
        clearForm();
        return;
    }

    int row = index.row();
    ui->comboBoxUserID->setCurrentText(m_model->data(m_model->index(row, 1)).toString());
    ui->lineEditUserName->setText(m_model->data(m_model->index(row, 2)).toString());
    ui->lineEditReservationID->setText(m_model->data(m_model->index(row, 0)).toString());
    ui->comboBoxReservationID->setCurrentText(m_model->data(m_model->index(row, 0)).toString());
    ui->lineEditAmount->setText(m_model->data(m_model->index(row, 3)).toString());
    ui->comboBoxPaymentStatus->setCurrentText(m_model->data(m_model->index(row, 4)).toString());
    ui->comboBoxPaymentMethod->setCurrentText(m_model->data(m_model->index(row, 5)).toString());
    QVariant dateValue = m_model->data(m_model->index(row, 6));
    ui->dateEditPaymentDate->setDate(dateValue.isNull() ? QDate::currentDate() : QDate::fromString(dateValue.toString(), "yyyy-MM-dd"));
    ui->lineEditTransactionID->setText(m_model->data(m_model->index(row, 7)).toString());
    ui->textEditPaymentNotes->setPlainText(m_model->data(m_model->index(row, 8)).toString());
    qDebug() << "Table row selected: Reservation ID" << m_model->data(m_model->index(row, 0)).toString();
}
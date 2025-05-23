#include "payments.h"
#include "ui_payments.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
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

    populateReservationComboBox();
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
            p.payment_id,
            p.reservation_id,
            p.amount,
            p.payment_method,
            p.payment_date
        FROM payments p
        WHERE 1=1
    )";

    if (!filter.isEmpty()) {
        queryStr += " AND CAST(p.reservation_id AS TEXT) LIKE :filter";
    }

    queryStr += " ORDER BY p.payment_date DESC";

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

    m_model->setHeaderData(0, Qt::Horizontal, "Payment ID");
    m_model->setHeaderData(1, Qt::Horizontal, "Reservation ID");
    m_model->setHeaderData(2, Qt::Horizontal, "Amount");
    m_model->setHeaderData(3, Qt::Horizontal, "Payment Method");
    m_model->setHeaderData(4, Qt::Horizontal, "Payment Date");

    ui->tablePayments->setColumnHidden(0, true);
    ui->tablePayments->setColumnWidth(1, 120);
    ui->tablePayments->setColumnWidth(2, 100);
    ui->tablePayments->setColumnWidth(3, 150);
    ui->tablePayments->setColumnWidth(4, 120);

    qDebug() << "Loaded" << m_model->rowCount() << "payments";
}

void PaymentsModule::populateReservationComboBox()
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
    query.prepare("SELECT reservation_id FROM reservations WHERE reservation_status = 'Confirmed' ORDER BY reservation_id");
    if (!query.exec()) {
        qDebug() << "Reservation query error:" << query.lastError().text();
        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        ui->comboBoxReservationID->addItem(query.value(0).toString());
    }
    qDebug() << "Populated" << ui->comboBoxReservationID->count() << "reservation IDs";
}

void PaymentsModule::clearForm()
{
    ui->lineEditPaymentID->clear();
    ui->comboBoxReservationID->setCurrentIndex(0);
    ui->lineEditAmount->clear();
    ui->comboBoxPaymentMethod->setCurrentIndex(0);
    ui->dateEditPaymentDate->setDate(QDate::currentDate());
    qDebug() << "Form cleared";
}

bool PaymentsModule::validateForm(int &reservationId, double &amount, QString &method, QDate &date)
{
    bool ok;
    reservationId = ui->comboBoxReservationID->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Input Error", "Invalid Reservation ID.");
        return false;
    }

    amount = ui->lineEditAmount->text().toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Input Error", "Amount must be a positive number.");
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

    return true;
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

    int reservationId;
    double amount;
    QString method;
    QDate date;
    if (!validateForm(reservationId, amount, method, date)) {
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
    QLabel label(QString("Add payment of $%1 for Reservation %2 via %3 on %4?")
                 .arg(amount, 0, 'f', 2)
                 .arg(reservationId)
                 .arg(method)
                 .arg(date.toString(Qt::ISODate)), &dialog);
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
        insertQuery.prepare("INSERT INTO payments (reservation_id, amount, payment_method, payment_date) "
                           "VALUES (:reservation_id, :amount, :payment_method, :payment_date)");
        insertQuery.bindValue(":reservation_id", reservationId);
        insertQuery.bindValue(":amount", amount);
        insertQuery.bindValue(":payment_method", method);
        insertQuery.bindValue(":payment_date", date);

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

        qDebug() << "Payment added: Reservation" << reservationId << amount << method << date;
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
    int paymentId = m_model->data(m_model->index(row, 0)).toInt();

    int reservationId;
    double amount;
    QString method;
    QDate date;
    if (!validateForm(reservationId, amount, method, date)) {
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
    QLabel label(QString("Update payment ID %1 to $%2 for Reservation %3 via %4 on %5?")
                 .arg(paymentId)
                 .arg(amount, 0, 'f', 2)
                 .arg(reservationId)
                 .arg(method)
                 .arg(date.toString(Qt::ISODate)), &dialog);
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
        updateQuery.prepare("UPDATE payments SET reservation_id = :reservation_id, amount = :amount, "
                           "payment_method = :payment_method, payment_date = :payment_date "
                           "WHERE payment_id = :payment_id");
        updateQuery.bindValue(":reservation_id", reservationId);
        updateQuery.bindValue(":amount", amount);
        updateQuery.bindValue(":payment_method", method);
        updateQuery.bindValue(":payment_date", date);
        updateQuery.bindValue(":payment_id", paymentId);

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

        qDebug() << "Payment updated: ID" << paymentId << reservationId << amount << method << date;
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
    int paymentId = m_model->data(m_model->index(row, 0)).toInt();

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Delete Payment");
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout layout(&dialog);
    QLabel label("Are you sure you want to delete this payment?", &dialog);
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
        deleteQuery.prepare("DELETE FROM payments WHERE payment_id = :payment_id");
        deleteQuery.bindValue(":payment_id", paymentId);

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

        qDebug() << "Payment deleted: ID" << paymentId;
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
    ui->lineEditPaymentID->setText(m_model->data(m_model->index(row, 0)).toString());
    ui->comboBoxReservationID->setCurrentText(m_model->data(m_model->index(row, 1)).toString());
    ui->lineEditAmount->setText(m_model->data(m_model->index(row, 2)).toString());
    ui->comboBoxPaymentMethod->setCurrentText(m_model->data(m_model->index(row, 3)).toString());
    ui->dateEditPaymentDate->setDate(QDate::fromString(m_model->data(m_model->index(row, 4)).toString(), Qt::ISODate));
    qDebug() << "Table row selected: Payment ID" << m_model->data(m_model->index(row, 0)).toString();
}

#include "customers.h"
#include "ui_customers.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QComboBox>
#include <QRegularExpression>

CustomersModule::CustomersModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomersModule),
    m_db(QSqlDatabase::database("main_connection")),
    m_model(new QSqlQueryModel(this))
{
    ui->setupUi(this);


    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }


    ui->customersTable->setModel(m_model);
    ui->customersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->customersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->customersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->customersTable->setSortingEnabled(true);
    ui->customersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->customersTable->verticalHeader()->hide();


    connect(ui->searchButton, &QPushButton::clicked, this, &CustomersModule::onSearchButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &CustomersModule::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &CustomersModule::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &CustomersModule::onDeleteButtonClicked);

    loadCustomers();
}

CustomersModule::~CustomersModule()
{
    delete m_model;
    delete ui;
}

void CustomersModule::loadCustomers(const QString &searchTerm)
{
    QString queryStr = R"(
        SELECT
            u.user_id AS "ID",
            u.full_name AS "Name",
            u.email AS "Email",
            u.phone_number AS "Phone",
            c.loyalty_points AS "Loyalty Points",
            c.preferred_room_type AS "Preferred Room Type",
            COUNT(r.reservation_id) AS "Reservations",
            ROUND(AVG(f.rating), 1) AS "Avg Rating"
        FROM users u
        JOIN customers c ON u.user_id = c.user_id
        LEFT JOIN reservations r ON u.user_id = r.user_id
        LEFT JOIN feedback f ON u.user_id = f.user_id
        WHERE u.role = 'Customer'
    )";

    if (!searchTerm.isEmpty()) {
        queryStr += " AND (u.full_name ILIKE '%' || :searchTerm || '%' OR u.email ILIKE '%' || :searchTerm || '%')";
    }
    queryStr += " GROUP BY u.user_id, u.full_name, u.email, u.phone_number, c.loyalty_points, c.preferred_room_type"
                " ORDER BY u.user_id";

    QSqlQuery query(m_db);
    query.prepare(queryStr);
    if (!searchTerm.isEmpty()) {
        query.bindValue(":searchTerm", searchTerm);
    }

    if (!query.exec()) {
        QMessageBox::critical(this, "Query Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    m_model->clear();
    m_model->setQuery(std::move(query));
    if (m_model->lastError().isValid()) {
        QMessageBox::critical(this, "Query Error", "Failed to load customers: " + m_model->lastError().text());
        return;
    }


    ui->customersTable->setModel(nullptr);
    ui->customersTable->setModel(m_model);
    ui->customersTable->viewport()->update();

    ui->customersTable->setColumnWidth(0, 80);  // ID
    ui->customersTable->setColumnWidth(1, 150); // Name
    ui->customersTable->setColumnWidth(2, 150); // Email
    ui->customersTable->setColumnWidth(3, 120); // Phone
    ui->customersTable->setColumnWidth(4, 100); // Loyalty Points
    ui->customersTable->setColumnWidth(5, 120); // Preferred Room Type
    ui->customersTable->setColumnWidth(6, 100); // Reservations
    ui->customersTable->setColumnWidth(7, 100); // Avg Rating
}

void CustomersModule::onSearchButtonClicked()
{
    QString searchTerm = ui->searchLineEdit->text().trimmed();
    loadCustomers(searchTerm);
}



void CustomersModule::onAddButtonClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add Customer");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *fullNameEdit = new QLineEdit(&dialog);
    QLineEdit *emailEdit = new QLineEdit(&dialog);
    QLineEdit *phoneEdit = new QLineEdit(&dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Optional");
    QLineEdit *loyaltyPointsEdit = new QLineEdit("0", &dialog);
    QComboBox *roomTypeCombo = new QComboBox(&dialog);
    roomTypeCombo->addItem("");
    QSqlQuery roomQuery(m_db);
    roomQuery.exec("SELECT DISTINCT room_type FROM rooms ORDER BY room_type");
    while (roomQuery.next()) {
        roomTypeCombo->addItem(roomQuery.value(0).toString());
    }

    formLayout->addRow("Full Name:", fullNameEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Phone Number:", phoneEdit);
    formLayout->addRow("Password:", passwordEdit);
    formLayout->addRow("Loyalty Points:", loyaltyPointsEdit);
    formLayout->addRow("Preferred Room Type:", roomTypeCombo);

    QPushButton *saveButton = new QPushButton("Save", &dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, [&]()

    {
        if (fullNameEdit->text().trimmed().isEmpty() || emailEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Full Name and Email are required.");
            return;
        }
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(emailEdit->text().trimmed()).hasMatch()) {
            QMessageBox::warning(this, "Input Error", "Invalid email format.");
            return;
        }
        bool ok;
        int points = loyaltyPointsEdit->text().toInt(&ok);
        if (!ok || points < 0) {
            QMessageBox::warning(this, "Input Error", "Loyalty Points must be a non-negative number.");
            return;
        }

        m_db.transaction();

        QSqlQuery query(m_db);
        query.prepare("INSERT INTO users (full_name, email, phone_number, role, password) "
                      "VALUES (:full_name, :email, :phone_number, 'Customer', :password) "
                      "RETURNING user_id");
        query.bindValue(":full_name", fullNameEdit->text().trimmed());
        query.bindValue(":email", emailEdit->text().trimmed());
        query.bindValue(":phone_number", phoneEdit->text().trimmed());
        query.bindValue(":password", passwordEdit->text().trimmed().isEmpty() ? QVariant() : passwordEdit->text().trimmed());

        if (!query.exec() || !query.next()) {
            m_db.rollback();
            QMessageBox::critical(this, "Error", "Failed to add user: " + query.lastError().text());
            return;
        }

        int userId = query.value(0).toInt();
        query.prepare("INSERT INTO customers (user_id, loyalty_points, preferred_room_type) "
                      "VALUES (:user_id, :loyalty_points, :preferred_room_type)");
        query.bindValue(":user_id", userId);
        query.bindValue(":loyalty_points", points);
        query.bindValue(":preferred_room_type", roomTypeCombo->currentText().isEmpty() ? QVariant() : roomTypeCombo->currentText());

        if (!query.exec()) {
            m_db.rollback();
            QMessageBox::critical(this, "Error", "Failed to add customer: " + query.lastError().text());
            return;
        }

        m_db.commit();
        dialog.accept();
        loadCustomers();
    });

    dialog.exec();
}



void CustomersModule::onEditButtonClicked()
{
    QModelIndexList selectedRows = ui->customersTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a customer to edit.");
        return;
    }

    int row = selectedRows[0].row();
    QString userId = m_model->data(m_model->index(row, 0)).toString();

    QSqlQuery query(m_db);
    query.prepare("SELECT u.full_name, u.email, u.phone_number, u.password, c.loyalty_points, c.preferred_room_type "
                  "FROM users u JOIN customers c ON u.user_id = c.user_id "
                  "WHERE u.user_id = :user_id AND u.role = 'Customer'");
    query.bindValue(":user_id", userId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to load customer data: " + query.lastError().text());
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Customer");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *fullNameEdit = new QLineEdit(query.value(0).toString(), &dialog);
    QLineEdit *emailEdit = new QLineEdit(query.value(1).toString(), &dialog);
    QLineEdit *phoneEdit = new QLineEdit(query.value(2).toString(), &dialog);
    QLineEdit *passwordEdit = new QLineEdit(query.value(3).toString(), &dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Optional");
    QLineEdit *loyaltyPointsEdit = new QLineEdit(query.value(4).toString(), &dialog);
    QComboBox *roomTypeCombo = new QComboBox(&dialog);
    roomTypeCombo->addItem("");
    QSqlQuery roomQuery(m_db);

    roomQuery.exec("SELECT DISTINCT room_type FROM rooms ORDER BY room_type");
    while (roomQuery.next()) {
        roomTypeCombo->addItem(roomQuery.value(0).toString());
    }
    roomTypeCombo->setCurrentText(query.value(5).toString());

    formLayout->addRow("Full Name:", fullNameEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Phone Number:", phoneEdit);
    formLayout->addRow("Password:", passwordEdit);
    formLayout->addRow("Loyalty Points:", loyaltyPointsEdit);
    formLayout->addRow("Preferred Room Type:", roomTypeCombo);

    QPushButton *saveButton = new QPushButton("Save", &dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, [&]()

    {
        if (fullNameEdit->text().trimmed().isEmpty() || emailEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Full Name and Email are required.");
            return;
        }


        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(emailEdit->text().trimmed()).hasMatch()) {
            QMessageBox::warning(this, "Input Error", "Invalid email format.");
            return;
        }

        bool ok;
        int points = loyaltyPointsEdit->text().toInt(&ok);

        if (!ok || points < 0) {
            QMessageBox::warning(this, "Input Error", "Loyalty Points must be a non-negative number.");
            return;
        }

        m_db.transaction();

        QSqlQuery updateQuery(m_db);
        updateQuery.prepare("UPDATE users SET full_name = :full_name, email = :email, phone_number = :phone_number, "
                           "password = :password WHERE user_id = :user_id AND role = 'Customer'");
        updateQuery.bindValue(":full_name", fullNameEdit->text().trimmed());
        updateQuery.bindValue(":email", emailEdit->text().trimmed());
        updateQuery.bindValue(":phone_number", phoneEdit->text().trimmed());
        updateQuery.bindValue(":password", passwordEdit->text().trimmed().isEmpty() ? QVariant() : passwordEdit->text().trimmed());
        updateQuery.bindValue(":user_id", userId);

        if (!updateQuery.exec()) {
            m_db.rollback();
            QMessageBox::critical(this, "Error", "Failed to update user: " + updateQuery.lastError().text());
            return;
        }

        updateQuery.prepare("UPDATE customers SET loyalty_points = :loyalty_points, preferred_room_type = :preferred_room_type "
                           "WHERE user_id = :user_id");
        updateQuery.bindValue(":loyalty_points", points);
        updateQuery.bindValue(":preferred_room_type", roomTypeCombo->currentText().isEmpty() ? QVariant() : roomTypeCombo->currentText());
        updateQuery.bindValue(":user_id", userId);

        if (!updateQuery.exec()) {
            m_db.rollback();
            QMessageBox::critical(this, "Error", "Failed to update customer: " + updateQuery.lastError().text());
            return;
        }

        m_db.commit();
        dialog.accept();
        loadCustomers();
    });

    dialog.exec();
}




void CustomersModule::onDeleteButtonClicked()
{
    QModelIndexList selectedRows = ui->customersTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a customer to delete.");
        return;
    }

    int row = selectedRows[0].row();
    QString userId = m_model->data(m_model->index(row, 0)).toString();

    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this customer?") == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM users WHERE user_id = :user_id AND role = 'Customer'");
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete customer: " + query.lastError().text());
            return;
        }
        loadCustomers();
    }
}

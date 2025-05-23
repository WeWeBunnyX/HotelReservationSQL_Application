#include "reports.h"
#include "ui_reports.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QDate>
#include <QDebug>

ReportsModule::ReportsModule(QWidget *parent)
    : QWidget(parent), ui(new Ui::Reports)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("main_connection");

    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::critical(this, "DB Error", "❌ Database is not connected or not open.");
        return;
    }

    connect(ui->showChartsButton, &QPushButton::clicked, this, &ReportsModule::showCharts);
}

ReportsModule::~ReportsModule()
{
    delete ui;
}

void ReportsModule::showCharts()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Hotel Reports");
    dialog.setModal(true);
    dialog.resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QTabWidget *tabWidget = new QTabWidget(&dialog);
    layout->addWidget(tabWidget);

    // Loyalty Points Histogram
    QChart *loyaltyChart = new QChart();
    QBarSeries *loyaltySeries = createLoyaltyPointsHistogram();
    if (loyaltySeries) {
        loyaltyChart->addSeries(loyaltySeries);
        loyaltyChart->createDefaultAxes();
        loyaltyChart->setTitle("Loyalty Points Distribution");
        QChartView *loyaltyChartView = new QChartView(loyaltyChart, this);
        setupChartView(loyaltyChartView, tabWidget, "Loyalty Points");
    }

    // Total Amount by Payment Method
    QChart *paymentMethodChart = new QChart();
    QBarSeries *paymentMethodSeries = createPaymentMethodBarSeries();
    if (paymentMethodSeries) {
        paymentMethodChart->addSeries(paymentMethodSeries);
        paymentMethodChart->createDefaultAxes();
        paymentMethodChart->setTitle("Total Amount by Payment Method");
        QChartView *paymentMethodChartView = new QChartView(paymentMethodChart, this);
        setupChartView(paymentMethodChartView, tabWidget, "Payment Methods");
    }

    // Payment Status Pie Chart
    QChart *paymentStatusChart = new QChart();
    QPieSeries *paymentStatusSeries = createPaymentStatusPieSeries();
    if (paymentStatusSeries) {
        paymentStatusChart->addSeries(paymentStatusSeries);
        paymentStatusChart->setTitle("Payment Status Distribution");
        QChartView *paymentStatusChartView = new QChartView(paymentStatusChart, this);
        setupChartView(paymentStatusChartView, tabWidget, "Payment Status");
    }

    // Reservations by Room Type
    QChart *reservationsChart = new QChart();
    QBarSeries *reservationsSeries = createReservationsByRoomTypeSeries();
    if (reservationsSeries) {
        reservationsChart->addSeries(reservationsSeries);
        reservationsChart->createDefaultAxes();
        reservationsChart->setTitle("Reservations by Room Type");
        QChartView *reservationsChartView = new QChartView(reservationsChart, this);
        setupChartView(reservationsChartView, tabWidget, "Reservations");
    }

    // Revenue by Month
    QChart *revenueChart = new QChart();
    QLineSeries *revenueSeries = createRevenueByMonthSeries();
    if (revenueSeries) {
        revenueChart->addSeries(revenueSeries);
        revenueChart->createDefaultAxes();
        revenueChart->setTitle("Revenue by Month");
        QChartView *revenueChartView = new QChartView(revenueChart, this);
        setupChartView(revenueChartView, tabWidget, "Revenue");
    }

    // Room Status Pie Chart
    QChart *roomStatusChart = new QChart();
    QPieSeries *roomStatusSeries = createRoomStatusPieSeries();
    if (roomStatusSeries) {
        roomStatusChart->addSeries(roomStatusSeries);
        roomStatusChart->setTitle("Room Status Distribution");
        QChartView *roomStatusChartView = new QChartView(roomStatusChart, this);
        setupChartView(roomStatusChartView, tabWidget, "Room Status");
    }

    QPushButton *saveButton = new QPushButton("Save as PNG", &dialog);
    layout->addWidget(saveButton);

    connect(saveButton, &QPushButton::clicked, [&]() {
        for (int i = 0; i < tabWidget->count(); ++i) {
            QChartView *view = qobject_cast<QChartView*>(tabWidget->widget(i));
            if (view) {
                QPixmap pixmap = view->grab();
                pixmap.save(QString("chart_%1.png").arg(tabWidget->tabText(i).toLower().replace(" ", "_")), "PNG");
            }
        }
        QMessageBox::information(&dialog, "Saved", "Charts saved as PNG files.");
    });

    dialog.exec();
}

QBarSeries* ReportsModule::createLoyaltyPointsHistogram()
{
    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("Loyalty Points");
    QStringList categories;
    QMap<QString, int> bins;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT loyalty_points FROM customers")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        int points = query.value(0).toInt();
        QString bin;
        if (points <= 50) bin = "0-50";
        else if (points <= 100) bin = "51-100";
        else if (points <= 200) bin = "101-200";
        else bin = ">200";
        bins[bin]++;
        if (!categories.contains(bin)) categories << bin;
    }

    if (bins.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (const QString &bin : categories) {
        *set << bins.value(bin, 0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    series->attachAxis(axisX);
    return series;
}

QBarSeries* ReportsModule::createPaymentMethodBarSeries()
{
    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("Total Amount");
    QStringList categories;
    QMap<QString, double> amounts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT payment_method, SUM(total_amount) AS total FROM reservations WHERE payment_method IS NOT NULL GROUP BY payment_method")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString method = query.value(0).toString();
        double amount = query.value(1).toDouble();
        amounts[method] = amount;
        categories << method;
    }

    if (amounts.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (const QString &method : categories) {
        *set << amounts.value(method, 0.0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    series->attachAxis(axisX);
    return series;
}

QPieSeries* ReportsModule::createPaymentStatusPieSeries()
{
    QPieSeries *series = new QPieSeries();
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT payment_status, COUNT(*) FROM reservations GROUP BY payment_status")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();
        counts[status] = count;
    }

    if (counts.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        series->append(it.key(), it.value());
    }
    return series;
}

QBarSeries* ReportsModule::createReservationsByRoomTypeSeries()
{
    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("Reservations");
    QStringList categories;
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT rm.room_type, COUNT(r.reservation_id) FROM reservations r JOIN rooms rm ON r.room_id = rm.room_id GROUP BY rm.room_type")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString roomType = query.value(0).toString();
        int count = query.value(1).toInt();
        counts[roomType] = count;
        categories << roomType;
    }

    if (counts.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (const QString &roomType : categories) {
        *set << counts.value(roomType, 0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    series->attachAxis(axisX);
    return series;
}

QLineSeries* ReportsModule::createRevenueByMonthSeries()
{
    QLineSeries *series = new QLineSeries();
    QMap<QString, double> revenue;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT TO_CHAR(check_in_date, 'YYYY-MM') AS month, SUM(total_amount) FROM reservations GROUP BY month ORDER BY month")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString month = query.value(0).toString();
        double amount = query.value(1).toDouble();
        revenue[month] = amount;
    }

    if (revenue.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (auto it = revenue.constBegin(); it != revenue.constEnd(); ++it) {
        series->append(QPointF(QDate::fromString(it.key() + "-01", "yyyy-MM-dd").toJulianDay(), it.value()));
    }
    return series;
}

QPieSeries* ReportsModule::createRoomStatusPieSeries()
{
    QPieSeries *series = new QPieSeries();
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT status, COUNT(*) FROM rooms GROUP BY status")) {
        qDebug() << "Query error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();
        counts[status] = count;
    }

    if (counts.isEmpty()) {
        delete series;
        return nullptr;
    }

    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        series->append(it.key(), it.value());
    }
    return series;
}

void ReportsModule::setupChartView(QChartView *chartView, QTabWidget *tabWidget, const QString &tabName)
{
    chartView->setRenderHint(QPainter::Antialiasing);
    tabWidget->addTab(chartView, tabName);
}

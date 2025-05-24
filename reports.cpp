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
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QDateTime>
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
        QBarCategoryAxis *axisX = nullptr;
        for (QAbstractAxis *axis : loyaltySeries->attachedAxes()) {
            if (axis->type() == QAbstractAxis::AxisTypeBarCategory) {
                axisX = qobject_cast<QBarCategoryAxis*>(axis);
                break;
            }
        }
        if (!axisX) {
            qDebug() << "Loyalty Points: Creating new X-axis";
            axisX = new QBarCategoryAxis();
            loyaltyChart->addAxis(axisX, Qt::AlignBottom);
            loyaltySeries->attachAxis(axisX);
        }
        axisX->setTitleText("Loyalty Points Range");
        qDebug() << "Loyalty Points X-Axis Categories:" << axisX->categories();
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Number of Customers");
        loyaltyChart->addAxis(axisY, Qt::AlignLeft);
        loyaltySeries->attachAxis(axisY);
        loyaltyChart->setTitle("Loyalty Points Distribution");
        QChartView *loyaltyChartView = new QChartView(loyaltyChart, this);
        setupChartView(loyaltyChartView, tabWidget, "Loyalty Points");
    } else {
        qDebug() << "Loyalty Points: No data to display";
        loyaltyChart->setTitle("Loyalty Points Distribution (No Data)");
        QChartView *loyaltyChartView = new QChartView(loyaltyChart, this);
        setupChartView(loyaltyChartView, tabWidget, "Loyalty Points");
    }

    // Total Amount by Payment Method
    QChart *paymentMethodChart = new QChart();
    QBarSeries *paymentMethodSeries = createPaymentMethodBarSeries();
    if (paymentMethodSeries) {
        paymentMethodChart->addSeries(paymentMethodSeries);
        QBarCategoryAxis *axisX = nullptr;
        for (QAbstractAxis *axis : paymentMethodSeries->attachedAxes()) {
            if (axis->type() == QAbstractAxis::AxisTypeBarCategory) {
                axisX = qobject_cast<QBarCategoryAxis*>(axis);
                break;
            }
        }
        if (!axisX) {
            qDebug() << "Payment Methods: Creating new X-axis";
            axisX = new QBarCategoryAxis();
            axisX->append({"Card", "Cash", "Online"});
            paymentMethodChart->addAxis(axisX, Qt::AlignBottom);
            paymentMethodSeries->attachAxis(axisX);
        }
        axisX->setTitleText("Payment Method");
        qDebug() << "Payment Methods X-Axis Categories:" << axisX->categories();
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Total Amount ($)");
        paymentMethodChart->addAxis(axisY, Qt::AlignLeft);
        paymentMethodSeries->attachAxis(axisY);
        paymentMethodChart->setTitle("Total Amount by Payment Method");
        QChartView *paymentMethodChartView = new QChartView(paymentMethodChart, this);
        setupChartView(paymentMethodChartView, tabWidget, "Payment Methods");
    } else {
        qDebug() << "Payment Methods: No data to display";
        paymentMethodChart->setTitle("Total Amount by Payment Method (No Data)");
        QChartView *paymentMethodChartView = new QChartView(paymentMethodChart, this);
        setupChartView(paymentMethodChartView, tabWidget, "Payment Methods");
    }

    // Payment Status Pie Chart
    QChart *paymentStatusChart = new QChart();
    QPieSeries *paymentStatusSeries = createPaymentStatusPieSeries();
    if (paymentStatusSeries) {
        paymentStatusChart->addSeries(paymentStatusSeries);
        for (QPieSlice *slice : paymentStatusSeries->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 (%2)").arg(slice->label()).arg(slice->value()));
        }
        paymentStatusChart->setTitle("Payment Status Distribution");
        QChartView *paymentStatusChartView = new QChartView(paymentStatusChart, this);
        setupChartView(paymentStatusChartView, tabWidget, "Payment Status");
    } else {
        qDebug() << "Payment Status: No data to display";
        paymentStatusChart->setTitle("Payment Status Distribution (No Data)");
        QChartView *paymentStatusChartView = new QChartView(paymentStatusChart, this);
        setupChartView(paymentStatusChartView, tabWidget, "Payment Status");
    }

    // Reservations by Room Type
    QChart *reservationsChart = new QChart();
    QBarSeries *reservationsSeries = createReservationsByRoomTypeSeries();
    if (reservationsSeries) {
        reservationsChart->addSeries(reservationsSeries);
        QBarCategoryAxis *axisX = nullptr;
        for (QAbstractAxis *axis : reservationsSeries->attachedAxes()) {
            if (axis->type() == QAbstractAxis::AxisTypeBarCategory) {
                axisX = qobject_cast<QBarCategoryAxis*>(axis);
                break;
            }
        }
        if (!axisX) {
            qDebug() << "Reservations: Creating new X-axis";
            axisX = new QBarCategoryAxis();
            axisX->append({"Basement Suite", "Deluxe", "Suite"});
            reservationsChart->addAxis(axisX, Qt::AlignBottom);
            reservationsSeries->attachAxis(axisX);
        }
        axisX->setTitleText("Room Type");
        qDebug() << "Reservations X-Axis Categories:" << axisX->categories();
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Number of Reservations");
        reservationsChart->addAxis(axisY, Qt::AlignLeft);
        reservationsSeries->attachAxis(axisY);
        reservationsChart->setTitle("Reservations by Room Type");
        QChartView *reservationsChartView = new QChartView(reservationsChart, this);
        setupChartView(reservationsChartView, tabWidget, "Reservations");
    } else {
        qDebug() << "Reservations: No data to display";
        reservationsChart->setTitle("Reservations by Room Type (No Data)");
        QChartView *reservationsChartView = new QChartView(reservationsChart, this);
        setupChartView(reservationsChartView, tabWidget, "Reservations");
    }

    // Revenue by Month
    QChart *revenueChart = new QChart();
    QLineSeries *revenueSeries = createRevenueByMonthSeries();
    if (revenueSeries) {
        revenueChart->addSeries(revenueSeries);
        QDateTimeAxis *axisX = new QDateTimeAxis();
        axisX->setFormat("MMM yyyy");
        axisX->setTitleText("Month");
        axisX->setTickCount(6); // Show ~6 months for readability
        // Set range to include data point (e.g., Jan 2025 to Dec 2025)
        QDateTime minDate(QDate(2025, 1, 1), QTime(0, 0), Qt::UTC);
        QDateTime maxDate(QDate(2025, 12, 31), QTime(0, 0), Qt::UTC);
        axisX->setRange(minDate, maxDate);
        revenueChart->addAxis(axisX, Qt::AlignBottom);
        revenueSeries->attachAxis(axisX);
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Revenue ($)");
        axisY->setLabelFormat("%.0f"); // Whole numbers for large values
        // Set range to fit $193,000
        axisY->setRange(0, 200000);
        revenueChart->addAxis(axisY, Qt::AlignLeft);
        revenueSeries->attachAxis(axisY);
        revenueChart->setTitle("Revenue by Month");
        QChartView *revenueChartView = new QChartView(revenueChart, this);
        revenueChartView->setMinimumSize(800, 600);
        revenueChartView->setRenderHint(QPainter::Antialiasing);
        setupChartView(revenueChartView, tabWidget, "Revenue");
        // Log axis ranges
        qDebug() << "Revenue X-Axis Range:" << axisX->min().toString("MMM yyyy") << "to" << axisX->max().toString("MMM yyyy");
        qDebug() << "Revenue Y-Axis Range:" << axisY->min() << "to" << axisY->max();
    } else {
        qDebug() << "Revenue: No data to display";
        revenueChart->setTitle("Revenue by Month (No Data)");
        QChartView *revenueChartView = new QChartView(revenueChart, this);
        revenueChartView->setMinimumSize(800, 600);
        setupChartView(revenueChartView, tabWidget, "Revenue");
    }

    // Room Status Pie Chart
    QChart *roomStatusChart = new QChart();
    QPieSeries *roomStatusSeries = createRoomStatusPieSeries();
    if (roomStatusSeries) {
        roomStatusChart->addSeries(roomStatusSeries);
        for (QPieSlice *slice : roomStatusSeries->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 (%2)").arg(slice->label()).arg(slice->value()));
        }
        roomStatusChart->setTitle("Room Status Distribution");
        QChartView *roomStatusChartView = new QChartView(roomStatusChart, this);
        setupChartView(roomStatusChartView, tabWidget, "Room Status");
    } else {
        qDebug() << "Room Status: No data to display";
        roomStatusChart->setTitle("Room Status Distribution (No Data)");
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
        qDebug() << "Loyalty Points Query Error:" << query.lastError().text();
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
        qDebug() << "Loyalty Points: Empty dataset";
        delete series;
        return nullptr;
    }

    for (const QString &bin : categories) {
        *set << bins.value(bin, 0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Loyalty Points Range");
    series->attachAxis(axisX);
    qDebug() << "Loyalty Points Series X-Axis Categories:" << axisX->categories();
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
        qDebug() << "Payment Method Query Error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    int rowCount = 0;
    while (query.next()) {
        QString method = query.value(0).toString();
        double amount = query.value(1).toDouble();
        qDebug() << "Payment Method:" << method << "Total:" << amount;
        amounts[method] = amount;
        if (!categories.contains(method)) {
            categories << method;
        }
        rowCount++;
    }

    qDebug() << "Payment Method: Retrieved" << rowCount << "rows with categories:" << categories;

    if (amounts.isEmpty()) {
        qDebug() << "Payment Method: Empty dataset";
        delete series;
        return nullptr;
    }

    for (const QString &method : categories) {
        *set << amounts.value(method, 0.0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Payment Method");
    series->attachAxis(axisX);
    qDebug() << "Payment Methods Series X-Axis Categories:" << axisX->categories();
    return series;
}

QPieSeries* ReportsModule::createPaymentStatusPieSeries()
{
    QPieSeries *series = new QPieSeries();
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT payment_status, COUNT(*) FROM reservations GROUP BY payment_status")) {
        qDebug() << "Payment Status Query Error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();
        counts[status] = count;
    }

    if (counts.isEmpty()) {
        qDebug() << "Payment Status: Empty dataset";
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
        qDebug() << "Reservations Query Error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    int rowCount = 0;
    while (query.next()) {
        QString roomType = query.value(0).toString();
        int count = query.value(1).toInt();
        qDebug() << "Reservations Room Type:" << roomType << "Count:" << count;
        counts[roomType] = count;
        if (!categories.contains(roomType)) {
            categories << roomType;
        }
        rowCount++;
    }

    qDebug() << "Reservations: Retrieved" << rowCount << "rows with categories:" << categories;

    if (counts.isEmpty()) {
        qDebug() << "Reservations: Empty dataset";
        delete series;
        return nullptr;
    }

    for (const QString &roomType : categories) {
        *set << counts.value(roomType, 0);
    }
    series->append(set);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Room Type");
    series->attachAxis(axisX);
    qDebug() << "Reservations Series X-Axis Categories:" << axisX->categories();
    return series;
}

QLineSeries* ReportsModule::createRevenueByMonthSeries()
{
    QLineSeries *series = new QLineSeries();
    series->setPointsVisible(true);
    QMap<QString, QPair<QDateTime, double>> revenue;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT TO_CHAR(check_in_date, 'YYYY-MM') AS month, SUM(total_amount) FROM reservations WHERE check_in_date IS NOT NULL GROUP BY month ORDER BY month")) {
        qDebug() << "Revenue Query Error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    int rowCount = 0;
    while (query.next()) {
        QString month = query.value(0).toString();
        double amount = query.value(1).toDouble();
        QDateTime date = QDate::fromString(month + "-01", "yyyy-MM-dd").startOfDay();
        if (date.isValid()) {
            revenue[month] = qMakePair(date, amount);
            qDebug() << "Revenue Month:" << month << "Amount:" << amount << "Date:" << date.toString("MMM yyyy");
        } else {
            qDebug() << "Revenue Invalid Date for month:" << month;
        }
        rowCount++;
    }

    qDebug() << "Revenue: Retrieved" << rowCount << "rows";

    if (revenue.isEmpty()) {
        qDebug() << "Revenue: Empty dataset";
        delete series;
        return nullptr;
    }

    for (auto it = revenue.constBegin(); it != revenue.constEnd(); ++it) {
        series->append(it.value().first.toMSecsSinceEpoch(), it.value().second);
        qDebug() << "Revenue Series Point:" << it.value().first.toString("MMM yyyy") << it.value().second;
    }

    return series;
}

QPieSeries* ReportsModule::createRoomStatusPieSeries()
{
    QPieSeries *series = new QPieSeries();
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT status, COUNT(*) FROM rooms GROUP BY status")) {
        qDebug() << "Room Status Query Error:" << query.lastError().text();
        delete series;
        return nullptr;
    }

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();
        counts[status] = count;
    }

    if (counts.isEmpty()) {
        qDebug() << "Room Status: Empty dataset";
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

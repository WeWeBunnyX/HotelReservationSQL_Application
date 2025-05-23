#ifndef REPORTS_H
#define REPORTS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QTabWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Reports; }
QT_END_NAMESPACE

class QSqlQuery;
class QBarSeries;
class QPieSeries;
class QLineSeries;

class ReportsModule : public QWidget
{
    Q_OBJECT

public:
    ReportsModule(QWidget *parent = nullptr);
    ~ReportsModule();

private slots:
    void showCharts();

private:
    Ui::Reports *ui;
    QSqlDatabase m_db;

    QBarSeries* createLoyaltyPointsHistogram();
    QBarSeries* createPaymentMethodBarSeries();
    QPieSeries* createPaymentStatusPieSeries();
    QBarSeries* createReservationsByRoomTypeSeries();
    QLineSeries* createRevenueByMonthSeries();
    QPieSeries* createRoomStatusPieSeries();

    void setupChartView(QChartView *chartView, QTabWidget *tabWidget, const QString &tabName);
};

#endif // REPORTS_H

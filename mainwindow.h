#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Dashboard;
class CustomersModule;
class RoomsModule;
class PaymentsModule;
class ReportsModule;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLinkActivated(const QString &link);

private:
    Ui::MainWindow *ui;
    Dashboard *dashboard;
    CustomersModule *customersModule;
    RoomsModule *roomsModule;
    PaymentsModule *paymentsModule;
    ReportsModule *reportsModule;
};

#endif // MAINWINDOW_H

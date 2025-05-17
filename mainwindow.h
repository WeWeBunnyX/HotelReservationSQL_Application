#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void openCustomers();
    void openRooms();
    void openPayments();
    void openReports();

private:
    Ui::MainWindow *ui;

    CustomersModule *customersWindow;
    RoomsModule *roomsWindow;
    PaymentsModule *paymentsWindow;
    ReportsModule *reportsWindow;
};

#endif // MAINWINDOW_H

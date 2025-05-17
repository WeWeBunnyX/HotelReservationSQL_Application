#ifndef REPORTS_H
#define REPORTS_H

#include <QWidget>

namespace Ui {
class ReportsModule;
}

class ReportsModule : public QWidget
{
    Q_OBJECT

public:
    explicit ReportsModule(QWidget *parent = nullptr);
    ~ReportsModule();

private:
    Ui::ReportsModule *ui;
};

#endif // REPORTS_H
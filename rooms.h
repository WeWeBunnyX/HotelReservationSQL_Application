#ifndef ROOMS_H
#define ROOMS_H

#include <QWidget>

namespace Ui {
class RoomsModule;
}

class RoomsModule : public QWidget
{
    Q_OBJECT

public:
    explicit RoomsModule(QWidget *parent = nullptr);
    ~RoomsModule();

private:
    Ui::RoomsModule *ui;
};

#endif // ROOMS_H
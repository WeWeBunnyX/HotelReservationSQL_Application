#ifndef ROOMS_H
#define ROOMS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
class RoomsModule;
}

class RoomsModule : public QWidget
{
    Q_OBJECT

public:
    explicit RoomsModule(QWidget *parent = nullptr);
    ~RoomsModule();

private slots:
    void on_addRoomButton_clicked();
    void on_editRoomButton_clicked();
    void on_deleteRoomButton_clicked();
    void on_searchLineEdit_textChanged(const QString &text);
    void on_roomsTable_clicked(const QModelIndex &index);

private:
    Ui::RoomsModule *ui;
    QSqlDatabase m_db;
    QSqlQueryModel *m_model;

    void loadRooms(const QString &filter = QString());
};

#endif // ROOMS_H

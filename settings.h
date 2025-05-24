#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

signals:
    void languageChanged(const QString &language);
    void themeChanged(bool isDark);
    void fontSizeChanged(int size);
    void autoSaveIntervalChanged(int interval);

private slots:
    void onSaveButtonClicked();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H

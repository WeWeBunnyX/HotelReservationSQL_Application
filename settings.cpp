#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    // Load saved settings
    QSettings settings("HotelReservation", "AppSettings");
    QString language = settings.value("language", "English").toString();
    bool isDark = settings.value("theme", false).toBool();
    QString fontSize = settings.value("fontSize", "Medium (16px)").toString();
    QString autoSave = settings.value("autoSave", "Off").toString();

    // Set UI elements to saved values
    ui->languageComboBox->setCurrentText(language);
    ui->fontSizeComboBox->setCurrentText(fontSize);
    ui->autoSaveComboBox->setCurrentText(autoSave);

    // Connect save button
    connect(ui->saveButton, &QPushButton::clicked, this, &Settings::onSaveButtonClicked);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::onSaveButtonClicked()
{
    QSettings settings("HotelReservation", "AppSettings");

    QString language = ui->languageComboBox->currentText();
    settings.setValue("language", language);
    emit languageChanged(language);

    QString fontSizeStr = ui->fontSizeComboBox->currentText();
    int fontSize = 16; // Default (Medium)
    if (fontSizeStr == "Small (12px)") fontSize = 12;
    else if (fontSizeStr == "Large (20px)") fontSize = 20;
    settings.setValue("fontSize", fontSizeStr);
    emit fontSizeChanged(fontSize);

    QString autoSaveStr = ui->autoSaveComboBox->currentText();
    int interval = 0; // Off
    if (autoSaveStr == "1 Minute") interval = 60;
    else if (autoSaveStr == "5 Minutes") interval = 300;
    settings.setValue("autoSave", autoSaveStr);
    emit autoSaveIntervalChanged(interval);

    // Show success message
    QMessageBox::information(this, "Success", "Settings applied successfully");
}

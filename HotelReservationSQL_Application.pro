QT += core gui widgets sql

CONFIG += c++11

TEMPLATE = app
TARGET = HotelReservationSQL_Application

SOURCES += \
    main.cpp \
    database.cpp \
    mainwindow.cpp

HEADERS += \
    database.h \
    mainwindow.h

FORMS += \
    mainwindow.ui



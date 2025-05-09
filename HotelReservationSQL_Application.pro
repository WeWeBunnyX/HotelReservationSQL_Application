QT += core gui widgets sql

CONFIG += c++11

TEMPLATE = app
TARGET = HotelReservationSQL_Application

SOURCES += \
    loginscreen.cpp \
    main.cpp \
    database.cpp \
    mainwindow.cpp

HEADERS += \
    database.h \
    loginscreen.h \
    mainwindow.h

FORMS += \
    loginscreen.ui \
    mainwindow.ui



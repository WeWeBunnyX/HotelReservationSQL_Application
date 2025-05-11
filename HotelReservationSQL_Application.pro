QT += core gui widgets sql

CONFIG += c++11

TEMPLATE = app
TARGET = HotelReservationSQL_Application

SOURCES += \
    loginscreen.cpp \
    main.cpp \
    database.cpp \
    mainwindow.cpp \
    registerscreen.cpp

HEADERS += \
    database.h \
    loginscreen.h \
    mainwindow.h \
    registerscreen.h

FORMS += \
    loginscreen.ui \
    mainwindow.ui \
    registerscreen.ui



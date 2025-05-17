QT += core gui widgets sql

CONFIG += c++11

TEMPLATE = app
TARGET = HotelReservationSQL_Application

SOURCES += \
    dashboard.cpp \
    loginscreen.cpp \
    main.cpp \
    database.cpp \
    mainwindow.cpp \
    registerscreen.cpp \
    reservations.cpp

HEADERS += \
    dashboard.h \
    database.h \
    loginscreen.h \
    mainwindow.h \
    registerscreen.h \
    reservations.h

FORMS += \
    dashboard.ui \
    loginscreen.ui \
    mainwindow.ui \
    registerscreen.ui \
    reservations.ui

RESOURCES += \
    resources.qrc



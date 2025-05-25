QT += core gui widgets sql charts

CONFIG += c++11

TEMPLATE = app
TARGET = HotelReservationSQL_Application

SOURCES += \
    customers.cpp \
    rooms.cpp \
    payments.cpp \
    reports.cpp \
 \
    dashboard.cpp \
    loginscreen.cpp \
    main.cpp \
    database.cpp \
    mainwindow.cpp \
    registerscreen.cpp \
    reservations.cpp \
    settings.cpp

HEADERS += \
    customers.h \
    rooms.h \
    payments.h \
    reports.h \
 \
    dashboard.h \
    database.h \
    loginscreen.h \
    mainwindow.h \
    registerscreen.h \
    reservations.h \
    settings.h

FORMS += \
    customers.ui \
    rooms.ui \
    payments.ui \
    reports.ui \
 \
    dashboard.ui \
    loginscreen.ui \
    mainwindow.ui \
    registerscreen.ui \
    reservations.ui \
    settings.ui

RESOURCES += \
    customers.cpp \
    rooms.cpp \
    payments.cpp \
    reports.cpp \
 \
    resources.qrc





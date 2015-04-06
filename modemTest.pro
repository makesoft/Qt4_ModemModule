#-------------------------------------------------
#
# Project created by QtCreator 2015-01-15T10:49:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testSMS
TEMPLATE = app


SOURCES += main.cpp\
    modem_module.cpp \
    modem_object.cpp

HEADERS  += \
    modem_module.h \
    modem_object.h

include(qextserialport/src/qextserialport.pri)

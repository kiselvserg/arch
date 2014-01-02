#-------------------------------------------------
#
# Project created by QtCreator 2013-12-02T22:26:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = arch
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    cpu.cpp

HEADERS  += mainwindow.h \
    cpu.h

FORMS    += mainwindow.ui

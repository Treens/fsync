#-------------------------------------------------
#
# Project created by QtCreator 2017-02-18T12:39:18
#
#-------------------------------------------------

QT	+= core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET	= fsync
TEMPLATE= app

DEFINES	+= QT_DEPRECATED_WARNINGS

INCLUDEPATH += "$$PWD/include"

VPATH += "$$PWD/src" "$$PWD/resources" "$$PWD/include"

SOURCES	+= fsyncwindow.cpp \
    ftree.cpp \
    main.cpp \
    applyworker.cpp \
    analyzeworker.cpp

HEADERS	+= fsyncwindow.h \
    ftree.h \
    applyworker.h \
    analyzeworker.h

FORMS	+= fsyncwindow.ui

DEFINES	+= "FSYNCVERSION='\"0.3.1\"'"

#-------------------------------------------------
#
# Project created by QtCreator 2016-10-19T19:27:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Imshow
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    defog.cpp

HEADERS  += dialog.h \
    defog.h

FORMS    += dialog.ui

include(Link/opencv_Link.pri)

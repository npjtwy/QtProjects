#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T03:20:46
#
#-------------------------------------------------


QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myQQ
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    toolbox1.cpp \
    logindlg.cpp

HEADERS  += widget.h \
    toolbox1.h \
    logindlg.h

RESOURCES += \
    myqq.qrc

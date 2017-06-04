TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    qqserver.cpp \
    pub.cpp \
    work.cpp \
    mysqlhelper.cpp \
    sqldemo.cpp \
    mysqlhelper.cpp

HEADERS += \
    pub.h \
    work.h \
    mysqlhelper.h

DISTFILES += \
    makefile_sql

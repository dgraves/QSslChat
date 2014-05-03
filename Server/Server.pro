#-------------------------------------------------
#
# Project created by QtCreator 2011-07-11T20:38:20
#
#-------------------------------------------------

QT       += core gui network widgets

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
        server.cpp \
    sslserver.cpp

HEADERS  += server.h \
    sslserver.h

FORMS    += server.ui

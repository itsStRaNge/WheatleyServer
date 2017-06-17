QT += core
QT += network
QT += widgets
QT -= gui

LIBS += -L"/usr/local/lib" -lwiringPi
CONFIG += c++11
QMAKE_CXXFLAGS += -lpthread -lwiringPi
TARGET = WheatleyServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp \
    RCSwitch.cpp

HEADERS += \
    server.h \
    RCSwitch.h

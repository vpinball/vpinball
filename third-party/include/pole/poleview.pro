QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = poleview

HEADERS += pole.h
HEADERS += poleview.h

SOURCES += pole.cpp
SOURCES += poleview.cpp

CONFIG += qt
CONFIG += thread 
CONFIG += warn_on
CONFIG += exceptions
CONFIG += release

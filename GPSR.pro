QT += core
QT -= gui

CONFIG += c++11

TARGET = GPSR
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    gpsr.cpp \


INCLUDEPATH += /usr/local/MATLAB/R2018b/extern/include/
INCLUDEPATH += /opt/intel/mkl/include
INCLUDEPATH += /usr/include/eigen3/
INCLUDEPATH += /usr/local/include

HEADERS += \
    functions.h
HEADERS += \
    wavelet2s.h

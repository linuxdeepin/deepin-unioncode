QT       -= gui

TARGET = QLogger
TEMPLATE = lib
CONFIG += static

include(QLogger.pri)

QMAKE_CXXFLAGS += -std=c++14

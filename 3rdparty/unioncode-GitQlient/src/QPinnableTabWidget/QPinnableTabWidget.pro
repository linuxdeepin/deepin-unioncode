QT       += core gui widgets

TARGET = QPinnableTabWidget

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = lib

include(QPinnableTabWidget.pri)

win32 {
    QMAKE_TARGET_PRODUCT = "$$TARGET"
    QMAKE_TARGET_COMPANY = "Cesc Software"
    QMAKE_TARGET_COPYRIGHT = "Francesc M."
} else:mac {
    QMAKE_TARGET_BUNDLE_PREFIX = "com.francescmm."
}

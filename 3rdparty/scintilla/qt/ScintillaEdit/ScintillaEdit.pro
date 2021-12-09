#-------------------------------------------------
#
# Project created by QtCreator 2011-05-05T12:41:23
#
#-------------------------------------------------

QT       += core gui

TARGET = ScintillaEdit
TEMPLATE = lib
CONFIG += lib_bundle
CONFIG += c++1z

CONFIG(release, debug|release) {
	DEFINES += NDEBUG=1
}

DESTDIR = ../../bin
DLLDESTDIR = ../../bin

macx {
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
}

include(ScintillaEdit.pri)

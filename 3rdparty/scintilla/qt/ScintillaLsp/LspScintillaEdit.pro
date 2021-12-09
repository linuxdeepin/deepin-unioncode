#
# Project for building an LSP-enabled editor
#

QT       += core gui

TARGET = LspScintillaEdit
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

include(ScintillaLsp.pri)

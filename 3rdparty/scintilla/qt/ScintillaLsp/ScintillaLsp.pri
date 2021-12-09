#
#	LSP support for Qt Scintilla
#

include(../ScintillaEdit/ScintillaEdit.pri)

CONFIG += lsp

SOURCES += \
	$${PWD}/qtlspclientobject.cpp \
	$${PWD}/LspScintillaEdit.cpp \
	$${PWD}/../ScintillaEditBase/ProcessQt.cpp \
	$${PWD}/../../lsp/ILspDocument.cpp \
	$${PWD}/../../lsp/lspclient.cpp \
	$${PWD}/../../lsp/lspclientlogic.cpp \
	$${PWD}/../../lsp/lspclientnotifier.cpp \
	$${PWD}/../../lsp/lspclientobject.cpp \
	$${PWD}/../../lsp/lspmessagehandler.cpp \
	$${PWD}/../../lsp/lspmessages.cpp \
	$${PWD}/../../lsp/lspprocess.cpp \
	$${PWD}/../../lsp/lspstructures.cpp \

HEADERS  += \
	$${PWD}/qtlspclientobject.h \
	$${PWD}/LspScintillaEdit.h \
	$${PWD}/../../lsp/lsp_dll.h \
	$${PWD}/../../lsp/ILspClientObject.h \
	$${PWD}/../../lsp/ILspDocument.h \
	$${PWD}/../../lsp/lspclient.h \
	$${PWD}/../../lsp/lspclientlogic.h \
	$${PWD}/../../lsp/lspclientnotifier.h \
	$${PWD}/../../lsp/lspclientobject.h \
	$${PWD}/../../lsp/lspmessagehandler.h \
	$${PWD}/../../lsp/lspmessages.h \
	$${PWD}/../../lsp/lspprocess.h \
	$${PWD}/../../lsp/lspstructures.h \
	$${PWD}/../../lsp/process.h \

DEFINES += SCI_LSP=1 SCI_LSP_MAKE_LIBRARY=1

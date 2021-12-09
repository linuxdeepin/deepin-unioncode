#
#	Includes for base Qt Scintilla editor
#

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 4.2.3

SOURCES += \
	$${PWD}/PlatQt.cpp \
	$${PWD}/ScintillaQt.cpp \
	$${PWD}/ScintillaEditBase.cpp \
	$${PWD}/../../src/XPM.cxx \
	$${PWD}/../../src/ViewStyle.cxx \
	$${PWD}/../../src/UniqueString.cxx \
	$${PWD}/../../src/UniConversion.cxx \
	$${PWD}/../../src/Style.cxx \
	$${PWD}/../../src/Selection.cxx \
	$${PWD}/../../src/ScintillaBase.cxx \
	$${PWD}/../../src/RunStyles.cxx \
	$${PWD}/../../src/RESearch.cxx \
	$${PWD}/../../src/PositionCache.cxx \
	$${PWD}/../../src/PerLine.cxx \
	$${PWD}/../../src/MarginView.cxx \
	$${PWD}/../../src/LineMarker.cxx \
	$${PWD}/../../src/KeyMap.cxx \
	$${PWD}/../../src/Indicator.cxx \
	$${PWD}/../../src/ExternalLexer.cxx \
	$${PWD}/../../src/EditView.cxx \
	$${PWD}/../../src/Editor.cxx \
	$${PWD}/../../src/EditModel.cxx \
	$${PWD}/../../src/Document.cxx \
	$${PWD}/../../src/Decoration.cxx \
	$${PWD}/../../src/DBCS.cxx \
	$${PWD}/../../src/ContractionState.cxx \
	$${PWD}/../../src/CharClassify.cxx \
	$${PWD}/../../src/CellBuffer.cxx \
	$${PWD}/../../src/Catalogue.cxx \
	$${PWD}/../../src/CaseFolder.cxx \
	$${PWD}/../../src/CaseConvert.cxx \
	$${PWD}/../../src/CallTip.cxx \
	$${PWD}/../../src/AutoComplete.cxx \
	$${PWD}/../../lexlib/WordList.cxx \
	$${PWD}/../../lexlib/StyleContext.cxx \
	$${PWD}/../../lexlib/PropSetSimple.cxx \
	$${PWD}/../../lexlib/LexerSimple.cxx \
	$${PWD}/../../lexlib/LexerNoExceptions.cxx \
	$${PWD}/../../lexlib/LexerModule.cxx \
	$${PWD}/../../lexlib/LexerBase.cxx \
	$${PWD}/../../lexlib/DefaultLexer.cxx \
	$${PWD}/../../lexlib/CharacterSet.cxx \
	$${PWD}/../../lexlib/Accessor.cxx \
	$${PWD}/../../lexlib/CharacterCategory.cxx \
	$$files($${PWD}/../../lexers/*.cxx, false)

HEADERS  += \
	$${PWD}/PlatQt.h \
	$${PWD}/ScintillaQt.h \
	$${PWD}/ScintillaEditBase.h \
	$${PWD}/../../src/XPM.h \
	$${PWD}/../../src/ViewStyle.h \
	$${PWD}/../../src/UniConversion.h \
	$${PWD}/../../src/Style.h \
	$${PWD}/../../src/SplitVector.h \
	$${PWD}/../../src/Selection.h \
	$${PWD}/../../src/ScintillaBase.h \
	$${PWD}/../../src/RunStyles.h \
	$${PWD}/../../src/RESearch.h \
	$${PWD}/../../src/PositionCache.h \
	$${PWD}/../../src/PerLine.h \
	$${PWD}/../../src/Partitioning.h \
	$${PWD}/../../src/LineMarker.h \
	$${PWD}/../../src/KeyMap.h \
	$${PWD}/../../src/Indicator.h \
	$${PWD}/../../src/FontQuality.h \
	$${PWD}/../../src/ExternalLexer.h \
	$${PWD}/../../src/Editor.h \
	$${PWD}/../../src/Document.h \
	$${PWD}/../../src/Decoration.h \
	$${PWD}/../../src/ContractionState.h \
	$${PWD}/../../src/CharClassify.h \
	$${PWD}/../../src/CellBuffer.h \
	$${PWD}/../../src/Catalogue.h \
	$${PWD}/../../src/CaseFolder.h \
	$${PWD}/../../src/CaseConvert.h \
	$${PWD}/../../src/CallTip.h \
	$${PWD}/../../src/AutoComplete.h \
	$${PWD}/../../include/Scintilla.h \
	$${PWD}/../../include/SciLexer.h \
	$${PWD}/../../include/Platform.h \
	$${PWD}/../../include/ILexer.h \
	$${PWD}/../../lexlib/WordList.h \
	$${PWD}/../../lexlib/StyleContext.h \
	$${PWD}/../../lexlib/SparseState.h \
	$${PWD}/../../lexlib/PropSetSimple.h \
	$${PWD}/../../lexlib/OptionSet.h \
	$${PWD}/../../lexlib/LexerSimple.h \
	$${PWD}/../../lexlib/LexerNoExceptions.h \
	$${PWD}/../../lexlib/LexerModule.h \
	$${PWD}/../../lexlib/LexerBase.h \
	$${PWD}/../../lexlib/LexAccessor.h \
	$${PWD}/../../lexlib/CharacterSet.h \
	$${PWD}/../../lexlib/CharacterCategory.h \
	$${PWD}/../../lexlib/Accessor.h


INCLUDEPATH += $${PWD}/../../include $${PWD}/../../src $${PWD}/../../lexlib

DEFINES += SCINTILLA_QT=1 MAKING_LIBRARY=1 SCI_LEXER=1 _CRT_SECURE_NO_DEPRECATE=1

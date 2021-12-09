cmake_minimum_required(VERSION 3.0.0)

project(qscintilla-lsp)

# Use Qt modules
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Setup the environment
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG=1)
endif()

add_definitions(
    -DSCI_LSP
    -DSCI_LSP_MAKE_LIBRARY
    -DSCINTILLA_QT
    -DMAKING_LIBRARY
    -DSCI_LEXER
    -D_CRT_SECURE_NO_DEPRECATE
    )

set(ScintillaLsp_HEADERS
    qtlspclientobject.h
    LspScintillaEdit.h
    ../../lsp/lsp_dll.h
    ../../lsp/ILspClientObject.h
    ../../lsp/ILspDocument.h
    ../../lsp/lspclient.h
    ../../lsp/lspclientlogic.h
    ../../lsp/lspclientnotifier.h
    ../../lsp/lspclientobject.h
    ../../lsp/lspmessagehandler.h
    ../../lsp/lspmessages.h
    ../../lsp/lspprocess.h
    ../../lsp/lspstructures.h
    ../../lsp/process.h
    )

set(ScintillaLsp_SOURCES
    qtlspclientobject.cpp
    LspScintillaEdit.cpp
    ../ScintillaEditBase/ProcessQt.cpp
    ../../lsp/ILspDocument.cpp
    ../../lsp/lspclient.cpp
    ../../lsp/lspclientlogic.cpp
    ../../lsp/lspclientnotifier.cpp
    ../../lsp/lspclientobject.cpp
    ../../lsp/lspmessagehandler.cpp
    ../../lsp/lspmessages.cpp
    ../../lsp/lspprocess.cpp
    ../../lsp/lspstructures.cpp
    )

set(ScintillaEdit_HEADERS
    ../ScintillaEdit/ScintillaEdit.h
    ../ScintillaEdit/ScintillaDocument.h
    )

set(ScintillaEdit_SOURCES
    ../ScintillaEdit/ScintillaEdit.cpp
    ../ScintillaEdit/ScintillaDocument.cpp
    )

set(ScintillaEditBase_HEADERS
    ../ScintillaEditBase/PlatQt.h
    ../ScintillaEditBase/ScintillaQt.h
    ../ScintillaEditBase/ScintillaEditBase.h
    ../../src/XPM.h
    ../../src/ViewStyle.h
    ../../src/UniConversion.h
    ../../src/Style.h
    ../../src/SplitVector.h
    ../../src/Selection.h
    ../../src/ScintillaBase.h
    ../../src/RunStyles.h
    ../../src/RESearch.h
    ../../src/PositionCache.h
    ../../src/PerLine.h
    ../../src/Partitioning.h
    ../../src/LineMarker.h
    ../../src/KeyMap.h
    ../../src/Indicator.h
    ../../src/FontQuality.h
    ../../src/ExternalLexer.h
    ../../src/Editor.h
    ../../src/Document.h
    ../../src/Decoration.h
    ../../src/ContractionState.h
    ../../src/CharClassify.h
    ../../src/CellBuffer.h
    ../../src/Catalogue.h
    ../../src/CaseFolder.h
    ../../src/CaseConvert.h
    ../../src/CallTip.h
    ../../src/AutoComplete.h
    ../../include/Scintilla.h
    ../../include/SciLexer.h
    ../../include/Platform.h
    ../../include/ILexer.h
    ../../lexlib/WordList.h
    ../../lexlib/StyleContext.h
    ../../lexlib/SparseState.h
    ../../lexlib/PropSetSimple.h
    ../../lexlib/OptionSet.h
    ../../lexlib/LexerSimple.h
    ../../lexlib/LexerNoExceptions.h
    ../../lexlib/LexerModule.h
    ../../lexlib/LexerBase.h
    ../../lexlib/LexAccessor.h
    ../../lexlib/CharacterSet.h
    ../../lexlib/CharacterCategory.h
    ../../lexlib/Accessor.h
    )

set(ScintillaEditBase_SOURCES
    ../ScintillaEditBase/PlatQt.cpp
    ../ScintillaEditBase/ScintillaQt.cpp
    ../ScintillaEditBase/ScintillaEditBase.cpp
    ../../src/XPM.cxx
    ../../src/ViewStyle.cxx
    ../../src/UniqueString.cxx
    ../../src/UniConversion.cxx
    ../../src/Style.cxx
    ../../src/Selection.cxx
    ../../src/ScintillaBase.cxx
    ../../src/RunStyles.cxx
    ../../src/RESearch.cxx
    ../../src/PositionCache.cxx
    ../../src/PerLine.cxx
    ../../src/MarginView.cxx
    ../../src/LineMarker.cxx
    ../../src/KeyMap.cxx
    ../../src/Indicator.cxx
    ../../src/ExternalLexer.cxx
    ../../src/EditView.cxx
    ../../src/Editor.cxx
    ../../src/EditModel.cxx
    ../../src/Document.cxx
    ../../src/Decoration.cxx
    ../../src/DBCS.cxx
    ../../src/ContractionState.cxx
    ../../src/CharClassify.cxx
    ../../src/CellBuffer.cxx
    ../../src/Catalogue.cxx
    ../../src/CaseFolder.cxx
    ../../src/CaseConvert.cxx
    ../../src/CallTip.cxx
    ../../src/AutoComplete.cxx
    ../../lexlib/WordList.cxx
    ../../lexlib/StyleContext.cxx
    ../../lexlib/PropSetSimple.cxx
    ../../lexlib/LexerSimple.cxx
    ../../lexlib/LexerNoExceptions.cxx
    ../../lexlib/LexerModule.cxx
    ../../lexlib/LexerBase.cxx
    ../../lexlib/DefaultLexer.cxx
    ../../lexlib/CharacterSet.cxx
    ../../lexlib/Accessor.cxx
    ../../lexlib/CharacterCategory.cxx
    )

file(GLOB LEX_SOURCE ../../lexers/*.cxx)

include_directories(
    ../../include
    ../../src
    ../../lexlib
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${LEX_SOURCE}
    ${ScintillaEditBase_HEADERS}
    ${ScintillaEditBase_SOURCES}
    ${ScintillaEdit_HEADERS}
    ${ScintillaEdit_SOURCES}
    ${ScintillaLsp_HEADERS}
    ${ScintillaLsp_SOURCES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

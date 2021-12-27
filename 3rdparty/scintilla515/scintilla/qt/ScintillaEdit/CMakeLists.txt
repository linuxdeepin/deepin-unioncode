project(scintillaedit)

include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla/include
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla/qt
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla/src
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla/qt/ScintillaEdit
    ${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla/qt/ScintillaEditBase
    )

set(SOURCES
    ScintillaEdit.cpp
    ScintillaDocument.cpp
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
    ../../src/Geometry.cxx
    ../../src/EditView.cxx
    ../../src/Editor.cxx
    ../../src/EditModel.cxx
    ../../src/Document.cxx
    ../../src/Decoration.cxx
    ../../src/DBCS.cxx
    ../../src/ContractionState.cxx
    ../../src/CharClassify.cxx
    ../../src/CharacterType.cxx
    ../../src/CharacterCategoryMap.cxx
    ../../src/CellBuffer.cxx
    ../../src/CaseFolder.cxx
    ../../src/CaseConvert.cxx
    ../../src/CallTip.cxx
    ../../src/AutoComplete.cxx
    )

set(HEADERS
    ScintillaEdit.h
    ScintillaDocument.h
    ../ScintillaEditBase/ScintillaEditBase.h
    ../ScintillaEditBase/ScintillaQt.h
    )


add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${HEADERS}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

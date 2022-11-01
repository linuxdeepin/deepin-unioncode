include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla/include
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla/qt
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla/src
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla/qt/ScintillaEdit
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/scintilla/qt/ScintillaEditBase
    )

set(CMAKE_CXX_STANDARD 17)

link_libraries(unioncode-scintillaedit)

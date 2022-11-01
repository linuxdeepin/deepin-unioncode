include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/lexilla
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/lexilla/include
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/lexilla/lexers
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/lexilla/lexlib
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515/lexilla/src
    )

set(CMAKE_CXX_STANDARD 17)

link_libraries(unioncode-lexilla)

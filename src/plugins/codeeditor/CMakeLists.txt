cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla515.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/lexilla514.cmake)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

set(CXX_CPP
    mainframe/naveditframe.cpp
    transceiver/sendevents.cpp
    transceiver/codeeditorreceiver.cpp
    textedittabwidget/scintillaeditextern.cpp
    textedittabwidget/textedittabwidget.cpp
    textedittabwidget/textedittabbar.cpp
    textedittabwidget/textedittitlebar.cpp
    textedittabwidget/textedit.cpp
    textedittabwidget/style/stylecolor.cpp
    textedittabwidget/style/stylesci.cpp
    textedittabwidget/style/stylelsp.cpp
    textedittabwidget/style/stylekeeper.cpp
    textedittabwidget/style/stylejsonfile.cpp
    textedittabwidget/style/language/stylelspcpp.cpp
    textedittabwidget/style/language/stylescicpp.cpp
    textedittabwidget/style/language/stylelspcmake.cpp
    textedittabwidget/style/language/stylescicmake.cpp
    codeeditor.cpp
    codeeditor.json
    )

set(CXX_H
    mainframe/naveditframe.h
    transceiver/sendevents.h
    transceiver/codeeditorreceiver.h
    textedittabwidget/scintillaeditextern.h
    textedittabwidget/textedittabwidget.h
    textedittabwidget/textedittitlebar.h
    textedittabwidget/textedittabbar.h
    textedittabwidget/textedit.h
    textedittabwidget/style/stylecolor.h
    textedittabwidget/style/stylekeeper.h
    textedittabwidget/style/stylesci.h
    textedittabwidget/style/stylelsp.h
    textedittabwidget/style/stylejsonfile.h
    textedittabwidget/style/language/stylelspcpp.h
    textedittabwidget/style/language/stylescicpp.h
    textedittabwidget/style/language/stylelspcmake.h
    textedittabwidget/style/language/stylescicmake.h
    codeeditor.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-common
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

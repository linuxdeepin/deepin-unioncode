cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/scintilla515.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/lexilla514.cmake)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

set(CXX_CPP
    mainframe/autohidedockwidget.cpp
    mainframe/naveditmainwindow.cpp
    transceiver/sendevents.cpp
    transceiver/codeeditorreceiver.cpp
    refactorwidget/codelentype.cpp
    refactorwidget/codelens.cpp
    refactorwidget/codelensdelegate.cpp
    refactorwidget/refactorwidget.cpp
    renamepopup/renamepopup.cpp
    textedittabwidget/scintillaeditextern.cpp
    textedittabwidget/textedittabwidget.cpp
    textedittabwidget/textedittabbar.cpp
    textedittabwidget/textedittitlebar.cpp
    textedittabwidget/texteditkeeper.cpp
    textedittabwidget/textedit.cpp
    textedittabwidget/style/stylecolor.cpp
    textedittabwidget/style/stylesci.cpp
    textedittabwidget/style/stylelsp.cpp
    textedittabwidget/style/stylejsonfile.cpp
    textedittabwidget/language/texteditcpp.cpp
    textedittabwidget/language/stylelspcpp.cpp
    textedittabwidget/language/stylescicpp.cpp
    textedittabwidget/language/texteditcmake.cpp
    textedittabwidget/language/stylelspcmake.cpp
    textedittabwidget/language/stylescicmake.cpp
    codeeditor.cpp
    codeeditor.json
    )

set(CXX_H
    mainframe/autohidedockwidget.h
    mainframe/naveditmainwindow.h
    transceiver/sendevents.h
    transceiver/codeeditorreceiver.h
    refactorwidget/codelentype.h
    refactorwidget/codelens.h
    refactorwidget/codelensdelegate.h
    refactorwidget/refactorwidget.h
    renamepopup/renamepopup.h
    textedittabwidget/scintillaeditextern.h
    textedittabwidget/textedittabwidget.h
    textedittabwidget/textedittitlebar.h
    textedittabwidget/textedittabbar.h
    textedittabwidget/texteditkeeper.h
    textedittabwidget/textedit.h
    textedittabwidget/style/stylecolor.h
    textedittabwidget/style/stylesci.h
    textedittabwidget/style/stylelsp.h
    textedittabwidget/style/stylejsonfile.h
    textedittabwidget/language/texteditcmake.h
    textedittabwidget/language/stylelspcmake.h
    textedittabwidget/language/stylescicmake.h
    textedittabwidget/language/texteditcpp.h
    textedittabwidget/language/stylelspcpp.h
    textedittabwidget/language/stylescicpp.h
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

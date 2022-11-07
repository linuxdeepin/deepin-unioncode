cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-scintilla515.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-lexilla514.cmake)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

set(CXX_CPP
    mainframe/autohidedockwidget.cpp
    mainframe/naveditmainwindow.cpp
    mainframe/toolbarmanager.cpp
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
    textedittabwidget/texteditsplitter.cpp
    textedittabwidget/style/lspclientkeeper.cpp
    textedittabwidget/style/stylecolor.cpp
    textedittabwidget/style/stylesci.cpp
    textedittabwidget/style/stylelsp.cpp
    textedittabwidget/style/stylejsonfile.cpp
    textedittabwidget/language/java/texteditjava.cpp
    textedittabwidget/language/java/stylelspjava.cpp
    textedittabwidget/language/java/stylescijava.cpp
    textedittabwidget/language/cpp/texteditcpp.cpp
    textedittabwidget/language/cpp/stylelspcpp.cpp
    textedittabwidget/language/cpp/stylescicpp.cpp
    textedittabwidget/language/cmake/texteditcmake.cpp
    textedittabwidget/language/cmake/stylelspcmake.cpp
    textedittabwidget/language/cmake/stylescicmake.cpp
    textedittabwidget/language/python/texteditpython.cpp
    textedittabwidget/language/python/stylelsppython.cpp
    textedittabwidget/language/python/stylescipython.cpp
    codeeditor.cpp
    codeeditor.json
    )

set(CXX_H
    mainframe/autohidedockwidget.h
    mainframe/naveditmainwindow.h
    mainframe/toolbarmanager.h
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
    textedittabwidget/texteditsplitter.h
    textedittabwidget/style/stylecolor.h
    textedittabwidget/style/stylesci.h
    textedittabwidget/style/stylelsp.h
    textedittabwidget/style/stylejsonfile.h
    textedittabwidget/style/lspclientkeeper.h
    textedittabwidget/language/java/texteditjava.h
    textedittabwidget/language/java/stylelspjava.h
    textedittabwidget/language/java/stylescijava.h
    textedittabwidget/language/cpp/texteditcpp.h
    textedittabwidget/language/cpp/stylelspcpp.h
    textedittabwidget/language/cpp/stylescicpp.h
    textedittabwidget/language/cmake/texteditcmake.h
    textedittabwidget/language/cmake/stylelspcmake.h
    textedittabwidget/language/cmake/stylescicmake.h
    textedittabwidget/language/python/texteditpython.h
    textedittabwidget/language/python/stylelsppython.h
    textedittabwidget/language/python/stylescipython.h
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

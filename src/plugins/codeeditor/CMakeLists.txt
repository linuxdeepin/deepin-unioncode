cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeeditor)

add_definitions(
    -DQSCINTILLA_DLL
    -DSCI_LEXER)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ../../../3rdparty/scintilla
    ../../../3rdparty/scintilla/qt
    ../../../3rdparty/scintilla/src
    ../../../3rdparty/scintilla/include
    ../../../3rdparty/scintilla/scintilla/lexlib
    )

set(CXX_CPP
    filelangdatabase.cpp
    naveditwidget.cpp
    sendevents.cpp
    codeeditorreceiver.cpp
    editfilestatusbar.cpp
    editfiletabwidget.cpp
    edittextwidget.cpp
    editwidget.cpp
    codeeditor.cpp
    codeeditor.json
    )

set(CXX_H
    filelangdatabase.h
    naveditwidget.h
    sendevents.h
    codeeditorreceiver.h
    editfilestatusbar.cpp
    editfiletabwidget.h
    edittextwidget.h
    editwidget.h
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
    qscintilla-lsp
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_PATH})

cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeedit)

add_definitions(-DQSCINTILLA_MAKE_DLL)

include_directories(
    ../../../3rdparty/qscintilla/scintilla/include
    ../../../3rdparty/qscintilla/scintilla/lexlib
    ../../../3rdparty/qscintilla/scintilla/src
    ../../../3rdparty/qscintilla/scintilla/src/Qsci
    )

set(CXX_CPP
    sendevents.cpp
    recvevents.cpp
    treenodewatcher.cpp
    treenode.cpp
    treeproxy.cpp
    treemenu.cpp
    treeviewselectionmodel.cpp
    treeviewdelegate.cpp
    treemodel.cpp
    treeview.cpp
    editwidget.cpp
    codeedit.cpp
    codeedit.json
    )

set(CXX_H
    sendevents.h
    recvevents.h
    treenodewatcher.h
    treenode.h
    treeproxy.h
    treemenu.h
    treeviewselectionmodel.h
    treeviewdelegate.h
    treemodel.h
    treeview.h
    editwidget.h
    codeedit.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    unioncode-base
    unioncode-service
#    qscintilla
    ${QtUseModules}
    ${PkgUserModules}
    )


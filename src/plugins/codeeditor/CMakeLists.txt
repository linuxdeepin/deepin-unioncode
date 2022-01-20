cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/scintilla515.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/scintilla515/lexilla514.cmake)

set(CXX_CPP
    filelangdatabase.cpp
    naveditwidget.cpp
    sendevents.cpp
    codeeditorreceiver.cpp
    editfilestatusbar.cpp
    editfiletabwidget.cpp
    edittextwidget.cpp
    edittextwidgetstyle.cpp
    editwidget.cpp
    codeeditor.cpp
    codeeditor.json
    )

set(CXX_H
    filelangdatabase.h
    naveditwidget.h
    sendevents.h
    codeeditorreceiver.h
    editfilestatusbar.h
    editfiletabwidget.h
    edittextwidget.h
    edittextwidgetstyle.h
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
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

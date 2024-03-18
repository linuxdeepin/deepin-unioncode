cmake_minimum_required(VERSION 3.0.2)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-jsonrpccpp.cmake)

project(performance)

configure_file(mainframe/config.h.in config.h)
include_directories(${PROJECT_BINARY_DIR})

set(CXX_CPP
    mainframe/perfflamegraphscripts.cpp
    mainframe/perfrecorddisplay.cpp
    mainframe/mainwindow.cpp
    mainframe/jsondispalysfmodel.cpp
    mainframe/jsondisplayview.cpp
    mainframe/jsondisplaymodel.cpp
    mainframe/client.cpp
    mainframe/jsontabwidget.cpp
    performance.cpp
    performance.json
    )

set(CXX_H
    mainframe/perfflamegraphscripts.h
    mainframe/perfrecorddisplay.h
    mainframe/mainwindow.h
    mainframe/jsondispalysfmodel.h
    mainframe/jsondisplayview.h
    mainframe/jsondisplaymodel.h
    mainframe/client.h
    mainframe/jsontabwidget.h
    performance.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    unioncode-jsonrpcclient
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



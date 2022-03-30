cmake_minimum_required(VERSION 3.0.2)

project(plugin-optioncmake)

set(CXX_CPP
    mainframe/kitsmanagerwidget.cpp
    mainframe/optioncmakegenerator.cpp
    mainframe/cmakeoptionwidget.cpp
    mainframe/kitmanager.cpp
    mainframe/kit.cpp
    mainframe/persistentsettings.cpp
    optioncmake.cpp
    optioncmake.json
    )

set(CXX_H
    mainframe/kitsmanagerwidget.h
    mainframe/optioncmakegenerator.h
    mainframe/cmakeoptionwidget.h
    mainframe/kitmanager.h
    mainframe/kit.h
    mainframe/persistentsettings.h
    optioncmake.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


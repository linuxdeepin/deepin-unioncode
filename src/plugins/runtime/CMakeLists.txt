cmake_minimum_required(VERSION 3.0.2)

project(plugin-runtime)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    environmentwidget.cpp
    configurewidget.cpp
    runtimewidget.cpp
    runtimeplugin.cpp
    runtime.json
    projectparser.cpp
    stepspane.cpp
    targetsmanager.cpp
    )

set(CXX_H
    environmentwidget.h
    configurewidget.h
    runtimewidget.h
    runtimeplugin.h
    projectparser.h
    stepspane.h
    targetsmanager.h
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
    Qt5::Xml
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

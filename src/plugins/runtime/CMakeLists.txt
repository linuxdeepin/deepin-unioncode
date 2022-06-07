cmake_minimum_required(VERSION 3.0.2)

project(plugin-runtime)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    environmentwidget.cpp
    runtimewidget.cpp
    runtimeplugin.cpp
    runtime.json
    projectparser.cpp
    stepspane.cpp
    targetsmanager.cpp
    overviewpane.cpp
    runconfigpane.cpp
    projectoptionpane.cpp
    runtime.json
    kitsmanagerwidget.cpp
    configureprojpane.cpp
    runtimemanager.cpp
    kitmanager.cpp
    environmentmanagerwidget.cpp
    shortcutsettingwidget.cpp
    kit.cpp
    persistentsettings.cpp
    )

set(CXX_H
    environmentwidget.h
    runtimewidget.h
    runtimeplugin.h
    projectparser.h
    stepspane.h
    targetsmanager.h
    overviewpane.h
    runconfigpane.h
    projectoptionpane.h
    kitsmanagerwidget.h
    configureprojpane.h
    runtimemanager.h
    kitmanager.cpp
    environmentmanagerwidget.h
    shortcutsettingwidget.h
    kit.h
    persistentsettings.h
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

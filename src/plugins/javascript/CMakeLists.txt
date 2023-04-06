cmake_minimum_required(VERSION 3.0.2)

project(plugin-javascript)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(javascript_CPP
    javascriptplugin.cpp
    jsgenerator.cpp
    javascriptplugin.json
    project/jsasynparse.cpp
    project/jsprojectgenerator.cpp
    project/properties/configpropertywidget.cpp
    project/properties/configutil.cpp
    )

set(javascript_H
    javascriptplugin.h
    jsgenerator.h
    project/jsasynparse.h
    project/jsprojectgenerator.h
    project/properties/configpropertywidget.h
    project/properties/configutil.h
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${javascript_CPP}
    ${javascript_H}
    ${QT_THEME}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


cmake_minimum_required(VERSION 3.0.2)

project(javascript)

find_package(Qt5 COMPONENTS Xml Script ScriptTools REQUIRED)

set(javascript_CPP
    javascriptplugin.cpp
    jsgenerator.cpp
    javascriptplugin.json
    project/jsprojectgenerator.cpp
    project/properties/configpropertywidget.cpp
    project/properties/configutil.cpp
    debugger/jsdebugger.cpp
    )

set(javascript_H
    javascriptplugin.h
    jsgenerator.h
    project/jsprojectgenerator.h
    project/properties/configpropertywidget.h
    project/properties/configutil.h
    debugger/jsdebugger.h
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${javascript_CPP}
    ${javascript_H}
    ${QT_THEME}
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    Qt5::Xml
    Qt5::Script
    Qt5::ScriptTools
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


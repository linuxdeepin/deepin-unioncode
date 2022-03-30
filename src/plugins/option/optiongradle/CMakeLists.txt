cmake_minimum_required(VERSION 3.0.2)

project(plugin-optiongradle)

set(CXX_CPP
    mainframe/gradlewidget.cpp
    mainframe/gradleoptionwidget.cpp
    mainframe/optiongradlegenerator.cpp
    optiongradle.cpp
    optiongradle.json
    )

set(CXX_H
    mainframe/gradlewidget.h
    mainframe/gradleoptionwidget.h
    mainframe/optiongradlegenerator.h
    optiongradle.h
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


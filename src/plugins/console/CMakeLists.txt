cmake_minimum_required(VERSION 3.0.2)

project(plugin-console)

find_package(QTermWidget5 REQUIRED)

set(CXX_CPP
    console.cpp
    console.json
    )

set(CXX_H
    console.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
        unioncode-framework
    unioncode-base
    unioncode-service
    qtermwidget5
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_PATH})


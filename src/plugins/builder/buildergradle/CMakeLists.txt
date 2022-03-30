cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildergradle)

set(CXX_CPP
    mainframe/gradlegenerator.cpp
    parser/gradleparser.cpp
    buildergradle.cpp
    buildergradle.json
    )

set(CXX_H
    mainframe/gradlegenerator.h
    parser/gradleparser.h
    buildergradle.h
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


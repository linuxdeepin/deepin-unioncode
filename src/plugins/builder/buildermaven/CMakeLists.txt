cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildermaven)

set(CXX_CPP
    mainframe/mavengenerator.cpp
    parser/mavenparser.cpp
    buildermaven.cpp
    buildermaven.json
    )

set(CXX_H
    mainframe/mavengenerator.h
    parser/mavenparser.h
    buildermaven.h
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


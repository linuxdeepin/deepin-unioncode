cmake_minimum_required(VERSION 3.0.2)

project(plugin-builderninja)

set(CXX_CPP
    mainframe/ninjagenerator.cpp
    parser/ninjaparser.cpp
    builderninja.cpp
    builderninja.json
    )

set(CXX_H
    mainframe/ninjagenerator.h
    parser/ninjaparser.h
    builderninja.h
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


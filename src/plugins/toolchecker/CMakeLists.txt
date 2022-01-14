cmake_minimum_required(VERSION 3.0.2)

project(plugin-toolchecker)

set(CXX_CPP
    toolchecker.cpp
    toolchecker.json
    )

set(CXX_H
    toolchecker.h
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
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



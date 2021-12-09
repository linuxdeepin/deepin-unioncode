cmake_minimum_required(VERSION 3.0.2)

project(plugin-debugger)

set(CXX_CPP
    debugger.cpp
    debugger.json
    )

set(CXX_H
    debugger.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
        unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_PATH})

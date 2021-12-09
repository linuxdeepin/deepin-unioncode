cmake_minimum_required(VERSION 3.0.2)

project(plugin-collaborators)

set(CXX_CPP
    collaborators.cpp
    collaborators.json
    )

set(CXX_H
    collaborators.h
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

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_PATH})



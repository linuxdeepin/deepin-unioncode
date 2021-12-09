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
    framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )



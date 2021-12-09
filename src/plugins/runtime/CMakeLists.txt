cmake_minimum_required(VERSION 3.0.2)

project(plugin-runtime)

set(CXX_CPP
    outputwidget.cpp
    processreader.cpp
    runtime.cpp
    runtime.json
    )

set(CXX_H
    outputwidget.h
    processreader.h
    runtime.h
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

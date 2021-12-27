cmake_minimum_required(VERSION 3.0.2)

project(plugin-runtime)

set(CXX_CPP
    environmentwidget.cpp
    configurewidget.cpp
    runtimewidget.cpp
    runtime.cpp
    runtime.json
    )

set(CXX_H
    environmentwidget.h
    configurewidget.h
    runtimewidget.h
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
    unioncode-common
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

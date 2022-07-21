cmake_minimum_required(VERSION 3.0.2)

project(plugin-optionjava)

set(CXX_CPP
    mainframe/jdkwidget.cpp
    mainframe/javaoptionwidget.cpp
    mainframe/optionjavagenerator.cpp
    optionjava.cpp
    optionjava.json
    )

set(CXX_H
    mainframe/jdkwidget.h
    mainframe/javaoptionwidget.h
    mainframe/optionjavagenerator.h
    optionjava.h
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
    unioncode-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildermaven)

set(CXX_CPP
    mainframe/mavengenerator.cpp
    mainframe/mavenmanager.cpp
    parser/mavenparser.cpp
    transceiver/mavenreceiver.cpp
    buildermaven.cpp
    buildermaven.json
    )

set(CXX_H
    mainframe/mavengenerator.h
    mainframe/mavenmanager.h
    parser/mavenparser.h
    transceiver/mavenreceiver.h
    buildermaven.h
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


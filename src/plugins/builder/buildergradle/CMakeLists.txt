cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildergradle)

set(CXX_CPP
    mainframe/gradlegenerator.cpp
    mainframe/gradlemanager.cpp
    transceiver/gradlereceiver.cpp
    buildergradle.cpp
    buildergradle.json
    )

set(CXX_H
    mainframe/gradlegenerator.h
    mainframe/gradlemanager.h
    transceiver/gradlereceiver.h
    buildergradle.h
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


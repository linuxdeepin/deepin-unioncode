cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectninja)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/ninjaasynparse.cpp
    mainframe/ninjagenerator.cpp
    projectninja.cpp
    projectninja.json
    )

set(CXX_H
    mainframe/ninjaasynparse.h
    mainframe/ninjagenerator.h
    projectninja.h
    )

set(CXX_QRC
    mainframe/ninja.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${CXX_QRC}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



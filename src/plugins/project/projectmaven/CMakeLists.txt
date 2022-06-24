cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectmaven)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/mavenasynparse.cpp
    mainframe/mavenitemkeeper.cpp
    mainframe/mavenopenhandler.cpp
    mainframe/mavengenerator.cpp
    transceiver/projectmavenreceiver.cpp
    transceiver/sendevents.cpp
    projectmaven.cpp
    projectmaven.json
    )

set(CXX_H
    mainframe/mavenasynparse.h
    mainframe/mavenitemkeeper.h
    mainframe/mavenopenhandler.h
    mainframe/mavengenerator.h
    transceiver/projectmavenreceiver.h
    transceiver/sendevents.h
    projectmaven.h
    )

set(CXX_QRC
    mainframe/maven.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${CXX_QRC}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectgradle)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/gradleasynparse.cpp
    mainframe/gradleitemkeeper.cpp
    mainframe/gradleopenhandler.cpp
    mainframe/gradlegenerator.cpp
    transceiver/projectgradlereceiver.cpp
    transceiver/sendevents.cpp
    projectgradle.cpp
    projectgradle.json
    )

set(CXX_H
    mainframe/gradleasynparse.h
    mainframe/gradleitemkeeper.h
    mainframe/gradleopenhandler.h
    mainframe/gradlegenerator.h
    transceiver/projectgradlereceiver.h
    transceiver/sendevents.h
    projectgradle.h
    )

set(CXX_QRC
    mainframe/gradle.qrc
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



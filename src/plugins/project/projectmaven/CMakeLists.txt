cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectmaven)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/mavenasynparse.cpp
    mainframe/mavengenerator.cpp
    mainframe/properties/configpropertywidget.cpp
    mainframe/properties/configutil.cpp
    projectmaven.cpp
    projectmaven.json
    )

set(CXX_H
    mainframe/mavenasynparse.h
    mainframe/mavengenerator.h
    mainframe/properties/configpropertywidget.h
    mainframe/properties/configutil.h
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
    framework
    base
    services
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



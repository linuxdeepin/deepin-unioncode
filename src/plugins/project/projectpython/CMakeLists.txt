cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectpython)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/pythonasynparse.cpp
    mainframe/pythongenerator.cpp
    mainframe/properties/configpropertywidget.cpp
    mainframe/properties/configutil.cpp
    projectpython.cpp
    projectpython.json
    )

set(CXX_H
    mainframe/pythonasynparse.h
    mainframe/pythongenerator.h
    mainframe/properties/configpropertywidget.h
    mainframe/properties/configutil.h
    projectpython.h
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



cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectpython)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/pythonasynparse.cpp
    mainframe/pythonitemkeeper.cpp
    mainframe/pythonopenhandler.cpp
    mainframe/pythongenerator.cpp
    transceiver/projectpythonreceiver.cpp
    transceiver/sendevents.cpp
    projectpython.cpp
    projectpython.json
    )

set(CXX_H
    mainframe/pythonasynparse.h
    mainframe/pythonitemkeeper.h
    mainframe/pythonopenhandler.h
    mainframe/pythongenerator.h
    transceiver/projectpythonreceiver.h
    transceiver/sendevents.h
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



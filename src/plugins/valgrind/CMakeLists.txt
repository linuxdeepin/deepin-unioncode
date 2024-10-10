cmake_minimum_required(VERSION 3.0.2)

project(valgrind)

set(CXX_CPP
    mainframe/valgrindrunner.cpp
    mainframe/valgrindbar.cpp
    mainframe/xmlstreamreader.cpp
    transceiver/valgrindreceiver.cpp
    valgrind.cpp
    valgrind.json
    )

set(CXX_H
    mainframe/valgrindrunner.h
    mainframe/valgrindbar.h
    mainframe/xmlstreamreader.h
    transceiver/valgrindreceiver.h
    valgrind.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

cmake_minimum_required(VERSION 3.0.2)

project(plugin-commandproxy)

set(CMAKE_INCLUDE_CURRENT_DIR true)

set(CXX_CPP
    transceiver/commandlinereceiver.cpp
    commandproxy.cpp
    commandexecuter.cpp
    )

set(CXX_H
    transceiver/commandlinereceiver.h
    commandproxy.h
    commandexecuter.h
    commandproxy.json
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUserModules}
    services
    common
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


cmake_minimum_required(VERSION 3.0.2)

project(plugin-container)

include(${CMAKE_SOURCE_DIR}/3rdparty/qtermwidget-0.14.1.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR true)

set(CXX_CPP
    mainframe/virtualtabwidget.cpp
    mainframe/virtualconsole.cpp
    mainframe/containerkeeper.cpp
    container.cpp
    container.json
    )

set(CXX_H
    mainframe/virtualtabwidget.h
    mainframe/virtualconsole.h
    mainframe/containerkeeper.h
    container.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


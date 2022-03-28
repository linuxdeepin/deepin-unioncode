cmake_minimum_required(VERSION 3.0.2)

project(plugin-collaborators)

include(${CMAKE_SOURCE_DIR}/3rdparty/GitQlient.cmake)

set(CXX_CPP
    mainframe/gitqlientwidget.cpp
    collaborators.cpp
    collaborators.json
    )

set(CXX_H
    mainframe/gitqlientwidget.h
    collaborators.h
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



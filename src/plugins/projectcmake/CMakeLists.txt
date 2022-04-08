cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectcmake)

set(CXX_CPP
    mainframe/cmakeopenhandler.cpp
    mainframe/cmakegenerator.cpp
    projectcmake.cpp
    projectcmake.json
    )

set(CXX_H
    mainframe/cmakeopenhandler.h
    mainframe/cmakegenerator.h
    projectcmake.h
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



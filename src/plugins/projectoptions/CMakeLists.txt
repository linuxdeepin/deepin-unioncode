cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectoptions)

set(CXX_CPP
    projoptionsplugin.cpp
    optionsdialog.cpp
    projectoptions.json
    )

set(CXX_H
    projoptionsplugin.h
    optionsdialog.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    unioncode-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

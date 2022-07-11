cmake_minimum_required(VERSION 3.0.2)

project(plugin-codeporting)

set(CXX_CPP
    codeporting.json
    codeportingmanager.cpp
    codeportingplugin.cpp
    configwidget.cpp
    )

set(CXX_H
    codeportingmanager.h
    codeportingplugin.h
    configwidget.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-service
    unioncode-base
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})
install(DIRECTORY porting-script/ DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/porting-script")


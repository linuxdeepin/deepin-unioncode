cmake_minimum_required(VERSION 3.0.2)

project(plugin-optionlsp)

set(CXX_CPP
    mainframe/adapterconfigure.cpp
    mainframe/managerlsptabwidget.cpp
    mainframe/optionlspgenerator.cpp
    optionlsp.cpp
    optionlsp.json
    )

set(CXX_H
    mainframe/adapterconfigure.h
    mainframe/managerlsptabwidget.h
    mainframe/optionlspgenerator.h
    optionlsp.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


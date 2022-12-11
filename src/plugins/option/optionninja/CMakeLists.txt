cmake_minimum_required(VERSION 3.0.2)

project(plugin-optionninja)

set(CXX_CPP
    mainframe/ninjawidget.cpp
    mainframe/ninjaoptionwidget.cpp
    mainframe/optionninjagenerator.cpp
    optionninja.cpp
    optionninja.json
    )

set(CXX_H
    mainframe/ninjawidget.h
    mainframe/ninjaoptionwidget.h
    mainframe/optionninjagenerator.h
    optionninja.h
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


cmake_minimum_required(VERSION 3.0.2)

project(plugin-optionmaven)

set(CXX_CPP
    mainframe/mavenwidget.cpp
    mainframe/mavenoptionwidget.cpp
    mainframe/optionmavengenerator.cpp
    optionmaven.cpp
    optionmaven.json
    )

set(CXX_H
    mainframe/mavenwidget.h
    mainframe/mavenoptionwidget.h
    mainframe/optionmavengenerator.h
    optionmaven.h
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


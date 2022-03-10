cmake_minimum_required(VERSION 3.0.2)

project(plugin-builder)

set(CXX_CPP
    builderplugin.cpp
    builder.json
    buildoutputpane.cpp
    project.cpp
    buildtarget.cpp
    builderreceiver.cpp
    buildmanager.cpp
    buildstep.cpp
    qmakestep.cpp
    makestep.cpp
    eventsender.cpp
    menumanager.cpp
    )

set(CXX_H
    builderplugin.h
    buildoutputpane.h
    project.h
    buildtarget.h
    builderreceiver.h
    buildmanager.h
    buildstep.h
    qmakestep.h
    makestep.h
    builderglobals.h
    buildersignals.h
    eventsender.h
    menumanager.h
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


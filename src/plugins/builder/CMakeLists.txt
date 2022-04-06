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
    tasks/ioutputparser.cpp
    tasks/cmakeparser.cpp
    tasks/fileutils.cpp
    tasks/task.cpp
    tasks/ansifilterparser.cpp
    tasks/gnumakeparser.cpp
    tasks/gccparser.cpp
    tasks/ldparser.cpp
    tasks/taskmodel.cpp
    tasks/taskmanager.cpp
    tasks/taskdelegate.cpp
    tasks/taskview.cpp
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
    tasks/ioutputparser.h
    tasks/cmakeparser.h
    tasks/qtcassert.h
    tasks/fileutils.h
    tasks/task.h
    tasks/ansifilterparser.h
    tasks/gnumakeparser.h
    tasks/gccparser.h
    tasks/ldparser.h
    tasks/taskmodel.h
    tasks/taskmanager.h
    tasks/taskdelegate.h
    tasks/taskview.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    tasks/resources.qrc
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


cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildercore)

set(CXX_CPP
    mainframe/buildoutputpane.cpp
    mainframe/project.cpp
    mainframe/buildtarget.cpp
    mainframe/buildmanager.cpp
    mainframe/buildstep.cpp
    mainframe/qmakestep.cpp
    mainframe/makestep.cpp
    mainframe/menumanager.cpp
    mainframe/commandstep.cpp
    mainframe/buildstep.cpp
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
    transceiver/eventreceiver.cpp
    transceiver/eventsender.cpp
    buildercore.cpp
    buildercore.json
    )

set(CXX_H
    mainframe/buildoutputpane.h
    mainframe/project.h
    mainframe/buildtarget.h
    mainframe/buildmanager.h
    mainframe/buildstep.h
    mainframe/qmakestep.h
    mainframe/makestep.h
    mainframe/builderglobals.h
    mainframe/buildersignals.h
    mainframe/menumanager.h
    mainframe/commandstep.h
    mainframe/buildstep.h
    tasks/ioutputparser.h
    tasks/cmakeparser.h
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
    transceiver/eventreceiver.h
    transceiver/eventsender.h    
    buildercore.h
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


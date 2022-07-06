cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildercmake)

set(CXX_CPP
    mainframe/project.cpp
    mainframe/buildtarget.cpp
    mainframe/buildmanager.cpp
    mainframe/buildstep.cpp
    mainframe/commandstep.cpp
    mainframe/buildstep.cpp
    mainframe/cmakegenerator.cpp
    mainframe/cmakemanager.cpp
    parser/cmakeparser.cpp
    parser/ansifilterparser.cpp
    parser/gnumakeparser.cpp
    parser/gccparser.cpp
    parser/ldparser.cpp
    transceiver/eventreceiver.cpp
    buildercmake.cpp
    buildercmake.json
    )

set(CXX_H
    mainframe/project.h
    mainframe/buildtarget.h
    mainframe/buildmanager.h
    mainframe/buildstep.h
    mainframe/commandstep.h
    mainframe/buildstep.h
    mainframe/cmakegenerator.h
    mainframe/cmakemanager.h
    parser/cmakeparser.h
    parser/ansifilterparser.h
    parser/gnumakeparser.h
    parser/gccparser.h
    parser/ldparser.h
    transceiver/eventreceiver.h
    buildercmake.h
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


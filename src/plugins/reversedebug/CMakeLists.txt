cmake_minimum_required(VERSION 3.0.2)

project(plugin-reversedebug)

set(CXX_CPP
    reversedebug.json
    reversedebugplugin.cpp
    eventsender.cpp
    minidumpruncontrol.cpp
    reversedebuggermgr.cpp
    )

set(CXX_H
    reversedebugplugin.h
    eventsender.h
    reversedebuggerconstants.h
    event_man.h
    minidumpruncontrol.h
    reversedebuggermgr.h
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
    unioncode-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


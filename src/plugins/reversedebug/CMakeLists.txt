cmake_minimum_required(VERSION 3.0.2)

project(plugin-reversedebug)

set(CXX_CPP
    reversedebug.json
    reversedebugplugin.cpp
    minidumpruncontrol.cpp
    reversedebuggermgr.cpp
    eventfilterdialog.cpp
    taskmodel.cpp
    taskfiltermodel.cpp
    timelinewidget.cpp
    taskwindow.cpp
    loadcoredialog.cpp
    )

set(CXX_H
    reversedebugplugin.h
    reversedebuggerconstants.h
    event_man.h
    minidumpruncontrol.h
    reversedebuggermgr.h
    eventfilterdialog.h
    taskmodel.h
    taskfiltermodel.h
    timelinewidget.h
    taskwindow.h
    loadcoredialog.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    resource.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    services
    base
    common
    eventman
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


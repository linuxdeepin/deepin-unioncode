cmake_minimum_required(VERSION 3.0.2)

project(plugin-workspace)

set(CXX_CPP
    workspace.cpp
    workspace.json
    workspacedata.cpp
    workspacereceiver.cpp
    workspaceobject.cpp
    workspacecmake.cpp
    processdialog.cpp
    sendevents.cpp
    )

set(CXX_H
    workspacecmake.h
    workspaceobject.h
    workspace.h
    workspacedata.h
    workspacereceiver.h
    processdialog.h
    sendevents.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
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



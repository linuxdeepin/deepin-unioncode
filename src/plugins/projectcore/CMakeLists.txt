cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectcore)

set(CXX_CPP
    mainframe/projecttreeview.cpp
    mainframe/projectkeeper.cpp
    mainframe/workspaceobject.cpp
    mainframe/workspacedata.cpp
    mainframe/workspacecmake.cpp
    transceiver/projectreceiver.cpp
    transceiver/sendevents.cpp
    projectcore.cpp
    projectcore.json
    )

set(CXX_H
    mainframe/projecttreeview.h
    mainframe/projectkeeper.h
    mainframe/workspaceobject.h
    mainframe/workspacedata.h
    mainframe/workspacecmake.h
    transceiver/projectreceiver.h
    transceiver/sendevents.h
    projectcore.h
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



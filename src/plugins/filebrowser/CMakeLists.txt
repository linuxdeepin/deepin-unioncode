cmake_minimum_required(VERSION 3.0.2)

project(plugin-filebrowser)

configure_file(config.h.in config.h)

set(CXX_CPP
    sendevents.cpp
    filebrowserreceiver.cpp
    treenodewatcher.cpp
    treenode.cpp
    treeproxy.cpp
    treemenu.cpp
    treeviewdelegate.cpp
    treemodel.cpp
    treeview.cpp
    filebrowser.cpp
    filebrowser.json
    )

set(CXX_H
    sendevents.h
    filebrowserreceiver.h
    treenodewatcher.h
    treenode.h
    treeproxy.h
    treemenu.h
    treeviewdelegate.h
    treemodel.h
    treeview.h
    filebrowser.h
    config.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-common
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

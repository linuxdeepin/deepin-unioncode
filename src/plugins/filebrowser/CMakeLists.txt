cmake_minimum_required(VERSION 3.0.2)

project(plugin-filebrowser)

set(CXX_CPP
    transceiver/sendevents.cpp
    transceiver/filebrowserreceiver.cpp
    mainframe/treenodewatcher.cpp
    mainframe/treenode.cpp
    mainframe/treeproxy.cpp
    mainframe/treemenu.cpp
    mainframe/treeviewdelegate.cpp
    mainframe/treemodel.cpp
    mainframe/treeview.cpp
    filebrowser.cpp
    filebrowser.json
    )

set(CXX_H
    transceiver/sendevents.h
    transceiver/filebrowserreceiver.h
    mainframe/treenodewatcher.h
    mainframe/treenode.h
    mainframe/treeproxy.h
    mainframe/treemenu.h
    mainframe/treeviewdelegate.h
    mainframe/treemodel.h
    mainframe/treeview.h
    filebrowser.h
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

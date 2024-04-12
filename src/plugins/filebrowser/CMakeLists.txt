cmake_minimum_required(VERSION 3.0.2)

project(filebrowser)

set(CXX_CPP
    transceiver/sendevents.cpp
    transceiver/filebrowserreceiver.cpp
    mainframe/treeviewkeeper.cpp
    mainframe/filetreeview.cpp
    filebrowser.cpp
    filebrowser.json
    )

set(CXX_H
    transceiver/sendevents.h
    transceiver/filebrowserreceiver.h
    mainframe/treeviewkeeper.h
    mainframe/filetreeview.h
    filebrowser.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    resource.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    common
    services
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

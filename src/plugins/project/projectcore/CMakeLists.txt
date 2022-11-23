cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectcore)

set(CXX_CPP
    mainframe/projectdelegate.cpp
    mainframe/projectselectionmodel.cpp
    mainframe/projectinfodialog.cpp
    mainframe/projecttreeview.cpp
    mainframe/projectkeeper.cpp
    transceiver/projectcorereceiver.cpp
    transceiver/sendevents.cpp
    projectcore.cpp
    projectcore.json
    )

set(CXX_H
    mainframe/projectdelegate.h
    mainframe/projectselectionmodel.h
    mainframe/projectinfodialog.h
    mainframe/projecttreeview.h
    mainframe/projectkeeper.h
    transceiver/projectcorereceiver.h
    transceiver/sendevents.h    
    projectcore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



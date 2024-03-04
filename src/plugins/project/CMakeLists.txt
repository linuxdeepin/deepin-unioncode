cmake_minimum_required(VERSION 3.0.2)

project(projectcore)

set(CXX_CPP
    mainframe/projectmodel.cpp
    mainframe/projectdelegate.cpp
    mainframe/projectselectionmodel.cpp
    mainframe/projectinfodialog.cpp
    mainframe/projecttree.cpp
    mainframe/projectkeeper.cpp
    locator/allprojectfilelocator.cpp
    locator/currentprojectlocator.cpp
    transceiver/projectcorereceiver.cpp
    transceiver/sendevents.cpp
    projectcore.cpp
    projectcore.json
    )

set(CXX_H
    mainframe/projectmodel.h
    mainframe/projectdelegate.h
    mainframe/projectselectionmodel.h
    mainframe/projectinfodialog.h
    mainframe/projecttree.h
    mainframe/projectkeeper.h
    transceiver/projectcorereceiver.h
    transceiver/sendevents.h
    locator/allprojectfilelocator.h
    locator/currentprojectlocator.h
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



cmake_minimum_required(VERSION 3.0.2)

project(projectcore)

set(CXX_CPP
    mainframe/projectmodel.cpp
    mainframe/projectdelegate.cpp
    mainframe/projectselectionmodel.cpp
    mainframe/projectinfodialog.cpp
    mainframe/projecttree.cpp
    mainframe/projectkeeper.cpp
    mainframe/recent/recentopenitemdelegate.cpp
    mainframe/recent/recentopenview.cpp
    mainframe/recent/recentopenwidget.cpp
    locator/allprojectfilelocator.cpp
    locator/currentprojectlocator.cpp
    locator/basefilelocator.cpp
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
    mainframe/recent/recentopenitemdelegate.h
    mainframe/recent/recentopenview.h
    mainframe/recent/recentopenwidget.h
    transceiver/projectcorereceiver.h
    transceiver/sendevents.h
    locator/allprojectfilelocator.h
    locator/currentprojectlocator.h
    locator/basefilelocator.h
    projectcore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    resource.qrc
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



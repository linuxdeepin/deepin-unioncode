cmake_minimum_required(VERSION 3.0.2)

project(recent)

set(CXX_CPP
    mainframe/displayrecentview.cpp
    mainframe/displayitemdelegate.cpp
    mainframe/recentdisplay.cpp
    transceiver/recentreceiver.cpp
    recent.cpp
    recent.json
    )

set(CXX_H
    mainframe/displayrecentview.h
    mainframe/displayitemdelegate.h
    mainframe/recentdisplay.h
    transceiver/recentreceiver.h
    recent.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    recent.qrc
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

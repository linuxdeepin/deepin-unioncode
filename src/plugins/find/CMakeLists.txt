cmake_minimum_required(VERSION 3.0.2)

project(find)

set(CXX_CPP
    transceiver/findreceiver.cpp
    findtoolbar.cpp
    findtoolwindow.cpp
    searchresultwindow.cpp
    findplugin.cpp
    findplugin.json
    )

set(CXX_H
    transceiver/findreceiver.h
    findtoolbar.h
    findtoolwindow.h
    searchresultwindow.h
    findplugin.h
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    find.qrc
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


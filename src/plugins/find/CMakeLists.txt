cmake_minimum_required(VERSION 3.0.2)

project(plugin-find)

set(CXX_CPP
    transceiver/findreceiver.cpp
    transceiver/findsender.cpp
    findtoolbar.cpp
    findtoolwindow.cpp
    searchresultwindow.cpp
    findplugin.cpp
    findplugin.json
    )

set(CXX_H
    transceiver/findreceiver.h
    transceiver/findsender.h
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


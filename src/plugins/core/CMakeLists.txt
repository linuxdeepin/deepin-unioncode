cmake_minimum_required(VERSION 3.0.2)

project(plugin-core)

set(CXX_CPP
    mainframe/windowkeeper.cpp
    mainframe/windowstatusbar.cpp
    mainframe/progressbar.cpp
    mainframe/plugindialog.cpp
    transceiver/corereceiver.cpp
    mainframe/loadingwidget.cpp
    mainframe/mainwindow.cpp
    core.cpp
    core.json
    )

set(CXX_H
    mainframe/windowkeeper.h
    mainframe/windowstatusbar.h
    mainframe/progressbar.h
    mainframe/plugindialog.h
    transceiver/corereceiver.h
    mainframe/loadingwidget.h
    mainframe/mainwindow.h
    core.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    core.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


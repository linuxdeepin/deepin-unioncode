cmake_minimum_required(VERSION 3.0.2)

project(plugin-core)

set(CXX_CPP
    mainframe/processdialog.cpp
    mainframe/windowtheme.cpp
    mainframe/windowkeeper.cpp
    mainframe/windowstatusbar.cpp
    mainframe/progressbar.cpp
    mainframe/optionsdialog.cpp
    mainframe/aboutdialog.cpp
    transceiver/sendevents.cpp
    transceiver/corereceiver.cpp
    core.cpp
    core.json
    )

set(CXX_H
    mainframe/processdialog.h
    mainframe/windowtheme.h
    mainframe/windowkeeper.h
    mainframe/windowstatusbar.h
    mainframe/progressbar.h
    mainframe/optionsdialog.h
    mainframe/aboutdialog.h
    transceiver/sendevents.h
    transceiver/corereceiver.h
    core.h
    )

set(QT_THEME
    theme/theme.qrc)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    core.qrc
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


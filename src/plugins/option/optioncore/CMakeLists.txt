cmake_minimum_required(VERSION 3.0.2)

project(optioncore)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP

    mainframe/shortcutsettingwidget.cpp
    mainframe/persistentsettings.cpp
    mainframe/optionsdialog.cpp
    mainframe/optiondefaultkeeper.cpp
    mainframe/profilesettingwidget.cpp

    mainframe/optionprofilesettinggenerator.cpp
    mainframe/optionshortcutsettinggenerator.cpp
    mainframe/navigationdelegate.cpp
    mainframe/shortcutedit.cpp
    optioncore.cpp
    optioncore.json
    )

set(CXX_H

    mainframe/shortcutsettingwidget.h
    mainframe/persistentsettings.h
    mainframe/optionsdialog.h
    mainframe/optiondefaultkeeper.h
    mainframe/profilesettingwidget.h

    mainframe/optionprofilesettinggenerator.h
    mainframe/optionshortcutsettinggenerator.h
    mainframe/navigationdelegate.h
    mainframe/shortcutedit.h
    optioncore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    optioncore.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    common
    services
    ${QtUseModules}
    ${PkgUserModules}
    Qt5::Xml
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

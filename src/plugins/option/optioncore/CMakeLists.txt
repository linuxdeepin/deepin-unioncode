cmake_minimum_required(VERSION 3.0.2)

project(plugin-optioncore)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/projectparser.cpp
    mainframe/environmentwidget.cpp
    mainframe/optioncmdgenerator.cpp
    mainframe/optionenvgenerator.cpp
    mainframe/shortcutsettingwidget.cpp
    mainframe/persistentsettings.cpp
    mainframe/optionsdialog.cpp
    mainframe/optiondefaultkeeper.cpp
    optioncore.cpp
    optioncore.json
    )

set(CXX_H
    mainframe/projectparser.h
    mainframe/environmentwidget.h
    mainframe/optioncmdgenerator.h
    mainframe/optionenvgenerator.h
    mainframe/shortcutsettingwidget.h
    mainframe/persistentsettings.h
    mainframe/optionsdialog.h
    mainframe/optiondefaultkeeper.h
    optioncore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-common
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    Qt5::Xml
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

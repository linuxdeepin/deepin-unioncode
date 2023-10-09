cmake_minimum_required(VERSION 3.0.2)

if(NOT PLUGIN_INSTALL_PATH)
    message(FATAL_ERROR "Please put this project into deepin-unioncode/src/plugins, and modify plugins/CMakeLists.txt to add it.")
endif()

project(plugin-demo)

set(CXX_CPP
    demo.cpp
    eventreceiver.cpp
    demo.json
    )

set(CXX_H
    demo.h
    eventreceiver.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    demo.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework # plug-in framework, must be set
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

# PLUGIN_INSTALL_PATH = /usr/lib/$Arch(x86_64-linux-gnu)/deepin-unioncode/plugins
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

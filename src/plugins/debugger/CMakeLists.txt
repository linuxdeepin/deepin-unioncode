cmake_minimum_required(VERSION 3.0.2)

project(plugin-debugger)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    debugger.json
    debuggerplugin.cpp
    debuggerrunparameters.cpp
    debugger.cpp
    debugsession.cpp
    eventsender.cpp
    eventreceiver.cpp
    debugmanager.cpp
    rawdebugsession.cpp
    runtimecfgprovider.cpp
    debugservice.cpp
    debugmodel.cpp
    appoutputpane.cpp
    )

set(CXX_H
    debuggerplugin.h
    debuggerrunparameters.h
    debugger.h
    debugsession.h
    debuggerglobals.h
    debuggersignals.h
    eventsender.h
    eventreceiver.h
    debugmanager.h
    rawdebugsession.h
    objects.hpp
    runtimecfgprovider.h
    debug.h
    debugservice.h
    debugmodel.h
    appoutputpane.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    unioncode-common
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

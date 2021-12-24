cmake_minimum_required(VERSION 3.0.2)

project(plugin-debugger)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    debugger.json
    debuggerplugin.cpp
    abstractdebugger.cpp
    debuggerrunparameters.cpp
    dapdebugger.cpp
    sessiondebug.cpp
    eventsender.cpp
    eventreceiver.cpp
    debugmanager.cpp
    rawdebugsession.cpp
    )

set(CXX_H
    debuggerplugin.h
    abstractdebugger.h
    debuggerrunparameters.h
    dapdebugger.h
    sessiondebug.h
    debuggerglobals.h
    debuggersignals.h
    eventsender.h
    eventreceiver.h
    debugmanager.h
    rawdebugsession.h
    objects.hpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

link_libraries(${CPPDAP_DIR}/lib/libcppdap.a)

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

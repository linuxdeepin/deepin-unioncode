cmake_minimum_required(VERSION 3.0.2)

project(plugin-debugger)

set(CXX_CPP
    debugger.json
    debuggerplugin.cpp
    abstractdebugger.cpp
    debuggerrunparameters.cpp
    dapdebugger.cpp
    sessiondebug.cpp
    )

set(CXX_H
    debuggerplugin.h
    abstractdebugger.h
    debuggerrunparameters.h
    dapdebugger.h
    sessiondebug.h
    debuggerglobals.h
    debuggersignals.h
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
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

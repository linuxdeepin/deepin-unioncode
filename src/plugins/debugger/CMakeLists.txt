cmake_minimum_required(VERSION 3.0.2)

project(debugger)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    debugger.json
    debuggerplugin.cpp
    debuggerrunparameters.cpp
    dap/dapdebugger.cpp
    dap/debugsession.cpp
    event/eventreceiver.cpp
    debugmanager.cpp
    dap/rawdebugsession.cpp
    runtimecfgprovider.cpp
    dap/debugservice.cpp
    dap/debugmodel.cpp
    interface/stackframemodel.cpp
    stackframe.cpp
    interface/localtreeitem.cpp
    interface/localtreemodel.cpp
    interface/stackframeview.cpp
    interface/breakpointitem.cpp
    breakpoint.cpp
    interface/breakpointmodel.cpp
    interface/messagebox.cpp
    interface/menumanager.cpp
    runner/runner.cpp
    )

set(CXX_H
    debuggerplugin.h
    debuggerrunparameters.h
    dap/dapdebugger.h
    dap/debugsession.h
    debuggerglobals.h
    debuggersignals.h
    event/eventreceiver.h
    debugmanager.h
    dap/rawdebugsession.h
    dap/objects.hpp
    runtimecfgprovider.h
    debug.h
    dap/debugservice.h
    dap/debugmodel.h
    interface/stackframemodel.h
    stackframe.h
    interface/localtreeitem.h
    interface/localtreemodel.h
    interface/variable.h
    interface/stackframeview.h
    interface/breakpointitem.h
    breakpoint.h
    interface/breakpointmodel.h
    interface/messagebox.h
    interface/menumanager.h
    runner/runner.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    resource.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

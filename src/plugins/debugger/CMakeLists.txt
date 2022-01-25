cmake_minimum_required(VERSION 3.0.2)

project(plugin-debugger)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    debugger.json
    debuggerplugin.cpp
    debuggerrunparameters.cpp
    dap/debugger.cpp
    dap/debugsession.cpp
    event/eventsender.cpp
    event/eventreceiver.cpp
    debugmanager.cpp
    dap/rawdebugsession.cpp
    runtimecfgprovider.cpp
    dap/debugservice.cpp
    dap/debugmodel.cpp
    interface/appoutputpane.cpp
    interface/stackframemodel.cpp
    stackframe.cpp
    interface/localtreeitem.cpp
    interface/localtreemodel.cpp
    interface/stackframeview.cpp
    )

set(CXX_H
    debuggerplugin.h
    debuggerrunparameters.h
    dap/debugger.h
    dap/debugsession.h
    debuggerglobals.h
    debuggersignals.h
    event/eventsender.h
    event/eventreceiver.h
    debugmanager.h
    dap/rawdebugsession.h
    dap/objects.hpp
    runtimecfgprovider.h
    debug.h
    dap/debugservice.h
    dap/debugmodel.h
    interface/appoutputpane.h
    interface/stackframemodel.h
    stackframe.h
    interface/localtreeitem.h
    interface/localtreemodel.h
    interface/variable.h
    interface/stackframeview.h
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

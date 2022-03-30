cmake_minimum_required(VERSION 3.0.2)

project(framework)

add_definitions(
    -DDPF_LIBRARY
    -DNO_BACKTRACE
    )

#set Module subdir
#   The module pattern cannot be broken, 
#   so the installation header file cannot be found
set(lifecycle ${CMAKE_CURRENT_SOURCE_DIR}/lifecycle)
set(listener ${CMAKE_CURRENT_SOURCE_DIR}/listener)
set(service ${CMAKE_CURRENT_SOURCE_DIR}/service)
set(event ${CMAKE_CURRENT_SOURCE_DIR}/event)
set(abus ${CMAKE_CURRENT_SOURCE_DIR}/abus)
set(log ${CMAKE_CURRENT_SOURCE_DIR}/log)
set(listener ${CMAKE_CURRENT_SOURCE_DIR}/listener)

set(LOBAL_H
    framework_global.h
    )

set(PRIVATE_H
    lifecycle/private/pluginmanager_p.h
    lifecycle/private/pluginmetaobject_p.h
    abus/private/appbus_p.h
    event/private/event_p.h
    )

set(PRIVATE_CPP
    lifecycle/private/pluginmanager_p.cpp
    abus/private/appbus_p.cpp
    )

set(PUBLIC_H
    event/event.h
    event/eventcallproxy.h
    event/eventhandler.h
    log/frameworklog.h
    log/codetimecheck.h
    log/logutils.h
    service/qtclassmanager.h
    service/qtclassfactory.h
    service/pluginservicecontext.h
    service/pluginservice.h
    lifecycle/pluginsetting.h
    lifecycle/pluginmetaobject.h
    lifecycle/plugin.h
    lifecycle/pluginmanager.h
    lifecycle/lifecycle.h
    lifecycle/plugindepend.h
    abus/appbus.h
    listener/private/listener_p.h
    listener/listener.h
    backtrace/backtrace.h
    framework.h
    )

set(PUBLIC_CPP
    event/event.cpp
    event/eventcallproxy.cpp
    event/eventhandler.cpp
    log/frameworklog.cpp
    log/codetimecheck.cpp
    log/logutils.cpp
    lifecycle/pluginsetting.cpp
    lifecycle/pluginmetaobject.cpp
    lifecycle/plugin.cpp
    lifecycle/pluginmanager.cpp
    lifecycle/lifecycle.cpp
    service/pluginservicecontext.cpp
    abus/appbus.cpp
    listener/listener.cpp
    backtrace/backtrace.cpp
    framework.cpp
    )

add_library(${PROJECT_NAME} SHARED
    ${LOBAL_H}
    ${PRIVATE_H}
    ${PRIVATE_CPP}
    ${PUBLIC_H}
    ${PUBLIC_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    stdc++fs
    ${QtUseModules}
    ${PkgUseModules}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
add_library(DFM::framework ALIAS ${PROJECT_NAME})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${LIBRARY_INSTALL_PREFIX}
    )

#recursion install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION
    ${INCLUDE_INSTALL_RPEFIX}
    FILES_MATCHING PATTERN "*.h"
    )

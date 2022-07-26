cmake_minimum_required(VERSION 3.0.2)

include(${CMAKE_SOURCE_DIR}/3rdparty/archive-jsonrpccpp.cmake)

project(languageadapter)

find_package(Qt5 CONFIG REQUIRED Core Gui Widgets Network Concurrent)


set(HEADERS
    logutils.h
    log.h
    backend.h
    protocol.h
    settinginfo.h
    setting.h
    route.h
    serverproxy.h
    )

set(SOURCES
    logutils.cpp
    log.cpp
    backend.cpp
    setting.cpp
    route.cpp
    serverproxy.cpp
    )

add_executable(${PROJECT_NAME} main.cpp
    ${HEADERS}
    ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    archive-jsonrpccpp-server
    archive-jsonrpccpp-common
    )

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")

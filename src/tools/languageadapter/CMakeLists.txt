cmake_minimum_required(VERSION 3.0.2)

project(languageadapter)

find_package(Qt5 CONFIG REQUIRED Core Network Concurrent)

set(HEADERS
    protocol.h
    settinginfo.h
    setting.h
    route.h
    handler.h
    handlerfrontend.h
    handlerbackend.h
    langserver.h
    )

set(SOURCES
    setting.cpp
    route.cpp
    handler.cpp
    handlerfrontend.cpp
    handlerbackend.cpp
    langserver.cpp
    )

add_executable(${PROJECT_NAME} main.cpp
    ${HEADERS}
    ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt5::Network
    )

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")

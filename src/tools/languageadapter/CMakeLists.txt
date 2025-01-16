cmake_minimum_required(VERSION 3.0.2)

project(languageadapter)

if ((NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "sw_64")
        AND (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
        AND (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
        AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
#    set(CMAKE_C_FLAGS "-fsanitize=thread")
#    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

if ((NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "sw_64")
        AND (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
        AND (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
        AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    #    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
    #    set(CMAKE_C_FLAGS "-fsanitize=thread")
    #    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Gui Widgets Network Concurrent)

set(HEADERS
    remotechecker.h
    jsonrpccallproxy.h
    )

set(SOURCES
    remotechecker.cpp
    jsonrpccallproxy.cpp
    )

add_executable(
    ${PROJECT_NAME}
    main.cpp
    ${HEADERS}
    ${SOURCES}
    )

target_link_libraries(${PROJECT_NAME}
    duc-common
    duc-framework
    )

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")
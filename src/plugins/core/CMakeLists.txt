cmake_minimum_required(VERSION 3.0.2)

project(core)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
    add_definitions(-DLOONGARCH64)
endif()

FILE(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_SOURCES}
    ${QT_THEME}
    core.qrc
)

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

if(NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
    # loogarch not support web engine now, maybe remove this condition later.
    find_package(Qt5 COMPONENTS WebEngineWidgets REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt5::WebEngineWidgets)
else()
    message(STATUS "Skipping Qt5::WebEngineWidgets for loongarch64 platform.")
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


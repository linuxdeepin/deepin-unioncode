cmake_minimum_required(VERSION 3.18)

project(core)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
    add_definitions(-DUNSUPPORTPLATFORM)
    message(STATUS "Skipping WebEngineWidgets for ${CMAKE_SYSTEM_PROCESSOR} platform.")
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
    duc-framework
    duc-base
    duc-services
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

if(NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebEngineWidgets REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

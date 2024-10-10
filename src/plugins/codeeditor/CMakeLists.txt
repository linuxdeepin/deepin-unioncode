cmake_minimum_required(VERSION 3.0.2)

project(codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-qscintilla214.cmake)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(CMARK REQUIRED libcmark)

FILE(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set (QRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/codeeditor.qrc"
)

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_SOURCES}
    ${QRC_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMARK_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    ${CMARK_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

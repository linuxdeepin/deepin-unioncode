cmake_minimum_required(VERSION 3.0.2)

project(optioncore)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Xml REQUIRED)

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
    optioncore.qrc
)

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    Qt${QT_VERSION_MAJOR}::Xml
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})
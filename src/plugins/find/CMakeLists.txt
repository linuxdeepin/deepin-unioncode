cmake_minimum_required(VERSION 3.0.2)

project(find)

FILE(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_SOURCES}
    ${QT_THEME}
    find.qrc
)

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


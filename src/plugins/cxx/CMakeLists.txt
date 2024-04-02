cmake_minimum_required(VERSION 3.0.2)

project(cxx)

find_package(Qt5 COMPONENTS Xml REQUIRED)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties")


FILE(GLOB CXX_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

set(CXX_QRC
    cmake/project/cmake.qrc
    ninja/project/ninja.qrc
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_FILES}
    ${QT_THEME}
    ${CXX_QRC}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


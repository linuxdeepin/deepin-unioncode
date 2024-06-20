cmake_minimum_required(VERSION 3.0.2)

project(cxx)

find_package(Qt5 COMPONENTS Xml REQUIRED)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties")

FILE(GLOB PROJECT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lexer/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lexer/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/builder/mainframe/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/builder/mainframe/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/project/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/project/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/option/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/option/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ninja/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/parser/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/parser/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/generator/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/builder/generator/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/configWidget/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/configWidget/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/bulidCfgWidget/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/bulidCfgWidget/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/runCfgWidget/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/properties/runCfgWidget/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/cbp/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/cbp/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/transceiver/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/project/transceiver/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/option/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/option/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

set(CXX_QRC
    cmake/project/cmake.qrc
    ninja/project/ninja.qrc
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_FILES}
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

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


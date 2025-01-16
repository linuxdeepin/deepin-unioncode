cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_CXX_STANDARD 17)

project(aimanager)

FILE(GLOB CODEGEEX_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME}
    SHARED
    ${CODEGEEX_FILES}
    aimanager.qrc
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )


if(NOT PLUGIN_INSTALL_PATH)
    set(PLUGIN_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/deepin-unioncode/plugins")
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



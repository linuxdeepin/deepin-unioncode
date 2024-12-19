cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_CXX_STANDARD 17)

project(aimanager)

find_package(duc-base REQUIRED)
find_package(duc-common REQUIRED)
find_package(duc-framework REQUIRED)
find_package(duc-services REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

set(QtFindModules Core Gui Widgets Concurrent)
foreach(QtModule ${QtFindModules})
    find_package(Qt5 COMPONENTS ${QtModule} REQUIRED)
    # include qt module private include directors
    include_directories(${Qt5${QtModule}_PRIVATE_INCLUDE_DIRS})
    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt5::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

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



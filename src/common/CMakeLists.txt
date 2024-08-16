project(common)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-jsonrpccpp.cmake)

install(FILES ${CMAKE_SOURCE_DIR}/src/common/resource/icons/ide.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)

configure_file(util/config.h.in config.h)
find_package(Qt5Network)

set(CMAKE_CXX_STANDARD 17)

add_definitions(
    -DCOMMON_LIBRARY
    )

FILE(GLOB COMMON_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${COMMON_FILES}
    resource/common.qrc
)
    
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    unioncode-jsonrpcclient
    Qt5::Network
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})

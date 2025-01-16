cmake_minimum_required(VERSION 3.0.2)

project(codeeditor)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-qscintilla214.cmake)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

find_package(ICU COMPONENTS i18n uc REQUIRED)
if(${QT_VERSION_MAJOR} MATCHES 6)
    find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(CMARK REQUIRED libcmark)
pkg_check_modules(CHARDET REQUIRED chardet)
pkg_check_modules(UCHARDET REQUIRED uchardet)

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
	${CHARDET_INCLUDE_DIRS}
	${UCHARDET_INCLUDE_DIRS}
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
    ${CHARDET_LIBRARIES}
    ${UCHARDET_LIBRARIES}
    ICU::i18n
    ICU::uc
)

if(${QT_VERSION_MAJOR} MATCHES 6)
target_link_libraries(${PROJECT_NAME}
    Qt6::Core5Compat
)
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

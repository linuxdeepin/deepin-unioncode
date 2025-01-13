cmake_minimum_required(VERSION 3.0.2)

project(codeeditor)

set(CMAKE_INCLUDE_CURRENT_DIR true)

add_definitions(-DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}")

find_package(ICU COMPONENTS i18n uc REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CMARK REQUIRED libcmark)
pkg_check_modules(CHARDET REQUIRED chardet)
pkg_check_modules(UCHARDET REQUIRED uchardet)

# qscintilla
set(QSCINTILLA_LIBRARY qscintilla2_qt5)
find_path(QSCINTILLA_INCLUDE_DIR qsciglobal.h
    HINTS ${Qt5Core_INCLUDE_DIRS}
    PATH_SUFFIXES Qsci
)

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
	${QSCINTILLA_INCLUDE_DIR}
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
    ${QSCINTILLA_LIBRARY}
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

cmake_minimum_required(VERSION 3.10)

project(unioncode-qscintilla)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    PrintSupport
    REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Qsci/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Qsci/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/*/*.cpp"
)

add_definitions(-DSCINTILLA_QT -DSCI_LEXER -DINCLUDE_DEPRECATED_FEATURES)

add_library(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${DtkWidget_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include
        ${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src
        ${CMAKE_CURRENT_SOURCE_DIR}/scintilla/lexlib
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Qsci
)

target_link_libraries(
    ${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    ${DtkWidget_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${LIBRARY_INSTALL_PREFIX})
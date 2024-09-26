cmake_minimum_required(VERSION 3.0.2)

project(codegeex)

find_package(KF5SyntaxHighlighting)

FILE(GLOB CODEGEEX_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

FILE(GLOB 3RD_DIFF_FILES
    "${CMAKE_SOURCE_DIR}/3rdparty/diff-match-patch/*.h"
    "${CMAKE_SOURCE_DIR}/3rdparty/diff-match-patch/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${CODEGEEX_FILES}
    ${3RD_DIFF_FILES}
    codegeex.qrc
)


target_link_libraries(${PROJECT_NAME}
    framework # plug-in framework, must be set
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    KF5::SyntaxHighlighting
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/3rdparty/diff-match-patch"
)

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

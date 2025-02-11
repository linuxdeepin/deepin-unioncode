cmake_minimum_required(VERSION 3.0.2)

project(chat)

find_package(KF5SyntaxHighlighting)
pkg_check_modules(CMARK REQUIRED libcmark)

FILE(GLOB CHAT_FILES
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
    ${CHAT_FILES}
    ${3RD_DIFF_FILES}
    chat.qrc
)


target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    KF5::SyntaxHighlighting
    ${CMARK_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/3rdparty/diff-match-patch"
        ${CMARK_INCLUDE_DIRS}
)

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

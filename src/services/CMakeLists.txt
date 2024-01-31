project(services)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)


FILE(GLOB SERVICES_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/debugger/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/debugger/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/project/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/project/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/option/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/option/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/builder/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/builder/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/language/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/language/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/terminal/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/locator/*.h"
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SERVICES_FILES}
    )

link_libraries(
    ${PROJECT_NAME}
    base
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

target_link_libraries(${PROJECT_NAME}
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

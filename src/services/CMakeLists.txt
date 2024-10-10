project(duc-services)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

add_definitions(
    -DSERVICES_LIBRARY
    )

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
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SERVICES_FILES}
    )

link_libraries(
    ${PROJECT_NAME}
    duc-base
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
#recursion install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION
    ${INCLUDE_INSTALL_RPEFIX}
    FILES_MATCHING PATTERN "*.h"
    )

# config pkgconfig file
configure_file(${CMAKE_SOURCE_DIR}/assets/dev/${PROJECT_NAME}/${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${CMAKE_SOURCE_DIR}/assets/dev/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

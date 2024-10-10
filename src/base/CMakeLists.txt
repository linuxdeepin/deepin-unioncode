project(duc-base)

FILE(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${PROJECT_SOURCES}
)

find_package(Dtk COMPONENTS Widget Core Gui REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})
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

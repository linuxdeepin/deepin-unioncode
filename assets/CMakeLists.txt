# install templates.
install(DIRECTORY templates/
    DESTINATION "${SOURCES_INSTALL_RPEFIX}/templates")
install(DIRECTORY models/
    DESTINATION "${SOURCES_INSTALL_RPEFIX}/models")

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools)

# install translation.
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    file(GLOB TS_FILES "translations/*.ts")
    foreach(TS_FILE IN LISTS TS_FILES)
        message(STATUS "process ts file: ${TS_FILE}")
        find_program(LUPDATE_COMMAND NAMES lupdate lupdate-Qt${QT_VERSION_MAJOR} lupdate-qt4 lupdate-pro)
        if (NOT LUPDATE_COMMAND)
            message(FATAL_ERROR "lupdate command not found")
        endif()
        execute_process(COMMAND ${LUPDATE_COMMAND} "${CMAKE_SOURCE_DIR}/src/" -ts -no-obsolete
            "${TS_FILE}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE OUTPUT
            ERROR_VARIABLE ERROR
            RESULT_VARIABLE RES)
        if (RES)
            message(FATAL_ERROR "process ts file result : ${RES}, with error: ${ERROR}")
        endif()
        message(STATUS "${OUTPUT}")
        execute_process(COMMAND lrelease ${TS_FILE})
    endforeach()
endif()

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION "${SOURCES_INSTALL_RPEFIX}/translations")

# install tools
set(TOOL_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${CMAKE_PROJECT_NAME}/tools/env")
message("tool install path: ${TOOL_INSTALL_PATH}")
file(GLOB TOOL_FILES "${CMAKE_SOURCE_DIR}/assets/tools/*.tar.gz")
message(STATUS "TOOL_FILES ${TOOL_FILES}")
install(FILES ${TOOL_FILES} DESTINATION ${TOOL_INSTALL_PATH})

# install configure files.
file(GLOB SUPPORTFILES "${CMAKE_CURRENT_SOURCE_DIR}/configures/*.support")
file(GLOB DESKTOPFILES "${CMAKE_CURRENT_SOURCE_DIR}/configures/*.desktop")
file(GLOB ICON "${CMAKE_CURRENT_SOURCE_DIR}/configures/*.svg")

install(FILES ${SUPPORTFILES} DESTINATION "${SOURCES_INSTALL_RPEFIX}/configures")
install(FILES ${DESKTOPFILES} DESTINATION "/usr/share/applications")
install(FILES ${ICON} DESTINATION "${SOURCES_INSTALL_RPEFIX}/configures/icons")
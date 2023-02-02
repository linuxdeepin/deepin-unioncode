install(DIRECTORY templates/
    DESTINATION "${SOURCES_INSTALL_RPEFIX}/templates")

find_package(Qt5 COMPONENTS LinguistTools)

file(GLOB TS_FILES "translations/*.ts")
foreach(TS_FILE IN LISTS TS_FILES)
    message(STATUS "process ts file: ${TS_FILE}")
    execute_process(COMMAND lupdate "${CMAKE_SOURCE_DIR}/src/" -ts -no-obsolete
        "${TS_FILE}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE ERROR
        RESULT_VARIABLE RES)
    #    if (RES)
    #        message(FATAL_ERROR "process ts file result : ${RES}, with error: ${ERROR}")
    #    endif()
    message(STATUS "${OUTPUT}")
    execute_process(COMMAND lrelease ${TS_FILE})
endforeach()

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION "${SOURCES_INSTALL_RPEFIX}/translations")

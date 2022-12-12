install(DIRECTORY templates/
    DESTINATION "${SOURCES_INSTALL_RPEFIX}/templates")

execute_process(COMMAND lupdate "${CMAKE_SOURCE_DIR}/src/" -ts -no-obsolete "${CMAKE_SOURCE_DIR}/assets/translations"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
file(GLOB TRANSLATION_FILES "translations/*.qm")

file(GLOB TS_FILES "translations/*.ts")
foreach(TS_FILE IN LISTS TS_FILES)
    message("process ts file: ${TS_FILE}")
    execute_process(COMMAND lrelease ${TS_FILE})
endforeach()

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION "${SOURCES_INSTALL_RPEFIX}/translations")

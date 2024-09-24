file(GLOB SHELLFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.sh")

install(
    FILES ${SHELLFILES}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts"
    )

install(DIRECTORY porting-script/
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/porting-script")

install(DIRECTORY action-analysis/
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/action-analysis")

install(DIRECTORY symbol-parser/
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/symbol-parser")

install(DIRECTORY prettyprinters/
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/prettyprinters")

install(DIRECTORY rag/
    DESTINATION "${LIBRARY_INSTALL_PREFIX}/scripts/rag")

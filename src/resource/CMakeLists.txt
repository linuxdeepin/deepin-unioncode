project(unioncode-resource)

set(configJson configure)

install(DIRECTORY ${configJson}
    DESTINATION
    ${SOURCES_INSTALL_RPEFIX})

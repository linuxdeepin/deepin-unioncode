set(CMAKE_INCLUDE_CURRENT_DIR ON)
# use outside framework from deepin framework
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(framework)
link_libraries(unioncode-framework)

add_subdirectory(app)
add_subdirectory(base)
add_subdirectory(common)
add_subdirectory(plugins)
add_subdirectory(services)
add_subdirectory(resource)
add_subdirectory(scripts)

install(DIRECTORY
    configures
    scripts
    DESTINATION
    ${SOURCES_INSTALL_RPEFIX})

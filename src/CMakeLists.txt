set(CMAKE_INCLUDE_CURRENT_DIR ON)
# use outside framework from deepin framework
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(framework)
link_libraries(framework)

add_subdirectory(app)
add_subdirectory(base)
add_subdirectory(common)
add_subdirectory(plugins)
add_subdirectory(services)
add_subdirectory(scripts)
add_subdirectory(tools)

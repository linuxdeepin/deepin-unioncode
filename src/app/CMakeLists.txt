project(app)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    )

add_executable(unioncode main.cpp)

target_link_libraries(
    unioncode
    ${QtUseModules}
    ${PkgUseModule}
    )

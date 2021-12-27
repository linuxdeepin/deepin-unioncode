project(unioncode)

configure_file(config.h.in config.h)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(
    unioncode
    unioncode-framework
    ${QtUseModules}
    ${PkgUseModule}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

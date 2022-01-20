project(unioncode)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(
    unioncode
    unioncode-framework
    unioncode-common
    ${QtUseModules}
    ${PkgUseModule}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

project(deepin-unioncode)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    framework
    common
    ${QtUseModules}
    ${PkgUseModule}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

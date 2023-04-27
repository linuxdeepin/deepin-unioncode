project(deepin-unioncode)

add_executable(${PROJECT_NAME} main.cpp commandparser.cpp commandparser.h)

target_link_libraries(
    ${PROJECT_NAME}
    framework
    common
    services
    ${QtUseModules}
    ${PkgUseModule}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

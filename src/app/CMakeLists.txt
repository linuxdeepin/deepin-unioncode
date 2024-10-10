project(deepin-unioncode)

add_executable(${PROJECT_NAME} main.cpp)

add_definitions(
    -DEXTEND_BACKTRACE
    )

target_link_libraries(
    ${PROJECT_NAME}
    duc-framework
    duc-common
    ${QtUseModules}
    ${PkgUseModule}
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

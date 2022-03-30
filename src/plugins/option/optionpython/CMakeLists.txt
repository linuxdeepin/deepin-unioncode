cmake_minimum_required(VERSION 3.0.2)

project(plugin-optionpython)

set(CXX_CPP
    mainframe/interperterwidget.cpp
    mainframe/pythonoptionwidget.cpp
    mainframe/optionpythongenerator.cpp
    optionpython.cpp
    optionpython.json
    )

set(CXX_H
    mainframe/interperterwidget.h
    mainframe/pythonoptionwidget.h
    mainframe/optionpythongenerator.h
    optionpython.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


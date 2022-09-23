cmake_minimum_required(VERSION 3.0.2)

project(plugin-toolchecker)

set(CXX_CPP
    mainframe/pip3dialog.cpp
    mainframe/dpkgdebchecker.cpp
    mainframe/processdialog.cpp
    mainframe/wgetdialog.cpp
    mainframe/backendchecker.cpp
    toolchecker.cpp
    toolchecker.json
    )

set(CXX_H
    mainframe/pip3dialog.h
    mainframe/dpkgdebchecker.h
    mainframe/processdialog.h
    mainframe/wgetdialog.h
    mainframe/backendchecker.h
    toolchecker.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



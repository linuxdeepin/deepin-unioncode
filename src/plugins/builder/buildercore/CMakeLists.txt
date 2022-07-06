cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildercore)

set(CXX_CPP
    tasks/taskmodel.cpp
    tasks/taskmanager.cpp
    tasks/taskdelegate.cpp
    tasks/taskview.cpp
    transceiver/buildersender.cpp
    mainframe/buildmanager.cpp
    mainframe/menumanager.cpp
    mainframe/compileoutputpane.cpp
    mainframe/problemoutputpane.cpp
    buildercore.cpp
    buildercore.json
    )

set(CXX_H
    tasks/taskmodel.h
    tasks/taskmanager.h
    tasks/taskdelegate.h
    tasks/taskview.h
    transceiver/buildersender.h
    mainframe/buildmanager.h
    mainframe/menumanager.h
    mainframe/compileoutputpane.h
    mainframe/problemoutputpane.h
    buildercore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    unioncode-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


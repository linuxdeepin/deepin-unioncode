cmake_minimum_required(VERSION 3.0.2)

project(plugin-buildercore)

set(CXX_CPP
    tasks/taskmodel.cpp
    tasks/taskmanager.cpp
    tasks/taskdelegate.cpp
    tasks/taskview.cpp
    transceiver/builderreceiver.cpp
    transceiver/buildersender.cpp
    mainframe/buildmanager.cpp
    mainframe/compileoutputpane.cpp
    mainframe/problemoutputpane.cpp
    mainframe/commonparser.cpp
    buildercore.cpp
    buildercore.json
    )

set(CXX_H
    tasks/taskmodel.h
    tasks/taskmanager.h
    tasks/taskdelegate.h
    tasks/taskview.h
    transceiver/builderreceiver.h
    transceiver/buildersender.h
    mainframe/buildmanager.h
    mainframe/compileoutputpane.h
    mainframe/problemoutputpane.h
    mainframe/commonparser.h
    buildercore.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    builder.qrc
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


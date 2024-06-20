cmake_minimum_required(VERSION 3.0.2)

project(actionanalyse)

set(CXX_CPP
    mainframe/configure.cpp
    mainframe/analysekeeper.cpp
    mainframe/wordcountanalyse.cpp
    transceiver/actionanalysereceiver.cpp
    actionanalyseplugin.cpp
    actionanalyseplugin.json
    )

set(CXX_H
    mainframe/configure.h
    mainframe/analysekeeper.h
    mainframe/wordcountanalyse.h
    transceiver/actionanalysereceiver.h
    actionanalyseplugin.h
    )


add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    common
    base
    services
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



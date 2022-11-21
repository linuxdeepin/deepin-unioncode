cmake_minimum_required(VERSION 3.0.2)

project(plugin-actionanalyse)

set(CXX_CPP
    mainframe/analysekeeper.cpp
    mainframe/wordcountanalyse.cpp
    transceiver/actionanalysereceiver.cpp
    actionanalyseplugin.cpp
    actionanalyseplugin.json
    )

set(CXX_H
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
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



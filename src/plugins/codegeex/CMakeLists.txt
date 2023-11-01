cmake_minimum_required(VERSION 3.0.2)

project(plugin-codegeex)

set(CXX_CPP
    codegeex.cpp
    eventreceiver.cpp
    codegeex/askapi.cpp
    codegeex/copilotapi.cpp
    askpage/askpage.cpp
    copilot.cpp
    codegeex.json
    )

set(CXX_H
    codegeex.h
    eventreceiver.h
    codegeex/askapi.h
    codegeex/copilotapi.h
    copilot.h
    askpage/askpage.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    codegeex.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework # plug-in framework, must be set
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

cmake_minimum_required(VERSION 3.0.2)

project(symbol)

find_package(Clang REQUIRED)
include_directories(${CLANG_INCLUDE_DIRS})

set(CXX_CPP
    mainframe/symbolmodel.cpp
    mainframe/symboltreeview.cpp
    mainframe/symbolkeeper.cpp
    mainframe/symbolparser.cpp
    transceiver/symbolreceiver.cpp
    clangparser/clangcursor.cpp
    clangparser/clangparser.cpp
    symbol.cpp
    symbol.json
    )

set(CXX_H
    mainframe/definitions.h
    mainframe/symbolmodel.h
    mainframe/symboltreeview.h
    mainframe/symbolkeeper.h
    mainframe/symbolparser.h
    transceiver/symbolreceiver.h
    clangparser/clangcursor.h
    clangparser/clangparser.h
    symbol.h
    )


add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    resource.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    libclang
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



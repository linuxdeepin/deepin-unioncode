cmake_minimum_required(VERSION 3.0.2)

project(plugin-symbol)

set(CXX_CPP
    mainframe/symboldelegate.cpp
    mainframe/symbolmodel.cpp
    mainframe/symboltreeview.cpp
    mainframe/symbolkeeper.cpp
    mainframe/symbolparser.cpp
    transceiver/symbolreceiver.cpp
    symbol.cpp
    symbol.json
    )

set(CXX_H
    mainframe/symboldelegate.h
    mainframe/symbolmodel.h
    mainframe/symboltreeview.h
    mainframe/symbolkeeper.h
    mainframe/symbolparser.h
    transceiver/symbolreceiver.h
    symbol.h
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
    unioncode-common
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



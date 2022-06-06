cmake_minimum_required(VERSION 3.0.2)

project(plugin-symbolcore)

set(CXX_CPP
    mainframe/symboldelegate.cpp
    mainframe/symboltreeview.cpp
    mainframe/symbolkeeper.cpp
    transceiver/symbolcorereceiver.cpp
    transceiver/sendevents.cpp
    symbolcore.cpp
    symbolcore.json
    )

set(CXX_H
    mainframe/symboldelegate.h
    mainframe/symboltreeview.h
    mainframe/symbolkeeper.h
    transceiver/symbolcorereceiver.h
    transceiver/sendevents.h
    symbolcore.h
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



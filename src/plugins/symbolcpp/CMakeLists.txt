cmake_minimum_required(VERSION 3.0.2)

project(plugin-symbolcpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(LibClang REQUIRED)
include_directories(${CLANG_INCLUDEDIR})

set(CXX_CPP
    mainframe/symbolicons.cpp
    mainframe/symbolcppasynparser.cpp
    mainframe/symbolcppgenerator.cpp
    transceiver/symbolcppreceiver.cpp
    transceiver/sendevents.cpp
    symbolcpp.cpp
    symbolcpp.json
    )

set(CXX_H
    mainframe/symbolicons.h
    mainframe/symbolcppasynparser.h
    mainframe/symbolcppgenerator.h
    transceiver/symbolcppreceiver.h
    transceiver/sendevents.h
    symbolcpp.h
    )

set(QSRC
    mainframe/symbolcpp.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QSRC}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    unioncode-common
    ${CLANG_LIBS}
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


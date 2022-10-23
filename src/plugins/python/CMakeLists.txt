cmake_minimum_required(VERSION 3.0.2)

project(plugin-python)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    python/pythondebug.cpp
    python/pythongenerator.cpp
    pythonplugin.cpp
    pythonplugin.json
    )

set(CXX_H
    python/pythondebug.h
    python/pythongenerator.h
    pythonplugin.h
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    unioncode-common
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


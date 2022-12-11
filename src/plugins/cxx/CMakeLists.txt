cmake_minimum_required(VERSION 3.0.2)

project(plugin-cxx)

set(CXX_CPP
    cmake/cmakegenerator.cpp
    cmake/cmakebuild.cpp
    cmake/cmakedebug.cpp
    ninja/ninjagenerator.cpp
    ninja/ninjabuild.cpp
    ninja/ninjadebug.cpp
    cxxplugin.cpp
    cxxplugin.json
    )

set(CXX_H
    cmake/cmakegenerator.h
    cmake/cmakebuild.h
    cmake/cmakedebug.h
    ninja/ninjagenerator.h
    ninja/ninjabuild.h
    ninja/ninjadebug.h
    cxxplugin.h
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
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


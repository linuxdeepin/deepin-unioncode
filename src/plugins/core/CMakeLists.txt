cmake_minimum_required(VERSION 3.0.2)

project(plugin-core)

set(CXX_CPP
    windowtheme.cpp
    windowswitcher.cpp
    windowkeeper.cpp
    sendevents.cpp
    core.cpp
    core.json
    )

set(CXX_H
    windowtheme.h
    windowswitcher.h
    windowkeeper.h
    sendevents.h
    core.h
    )

set(QT_THEME
    theme/theme.qrc)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )



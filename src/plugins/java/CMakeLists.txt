cmake_minimum_required(VERSION 3.0.2)

project(plugin-java)

set(CXX_CPP
    maven/mavengenerator.cpp
    maven/mavenbuild.cpp
    gradle/gradlegenerator.cpp
    gradle/gradlebuild.cpp
    java/javadebug.cpp
    javaplugin.cpp
    javaplugin.json
    )

set(CXX_H
    maven/mavengenerator.h
    maven/mavenbuild.h
    gradle/gradlegenerator.h
    gradle/gradlebuild.h
    java/javadebug.h
    javaplugin.h
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
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


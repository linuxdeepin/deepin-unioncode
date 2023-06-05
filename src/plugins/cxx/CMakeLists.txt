cmake_minimum_required(VERSION 3.0.2)

project(plugin-cxx)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    cmake/cmakegenerator.cpp
    cmake/cmakebuild.cpp
    cmake/cmakedebug.cpp
    ninja/ninjagenerator.cpp
    ninja/ninjabuild.cpp
    ninja/ninjadebug.cpp
    cmake/builder/mainframe/cmakebuildergenerator.cpp
    cmake/builder/parser/cmakeparser.cpp
    cmake/builder/parser/ansifilterparser.cpp
    cmake/builder/parser/gnumakeparser.cpp
    cmake/builder/parser/gccparser.cpp
    cmake/builder/parser/ldparser.cpp
    cmake/project/mainframe/cmakeasynparse.cpp
    cmake/project/mainframe/cmakeitemkeeper.cpp
    cmake/project/mainframe/cmakeprojectgenerator.cpp
    cmake/project/mainframe/properties/environmentwidget.cpp
    cmake/project/mainframe/properties/stepspane.cpp
    cmake/project/mainframe/properties/targetsmanager.cpp
    cmake/project/mainframe/properties/runconfigpane.cpp
    cmake/project/mainframe/properties/configureprojpane.cpp
    cmake/project/mainframe/properties/buildpropertywidget.cpp
    cmake/project/mainframe/properties/runpropertywidget.cpp
    cmake/project/mainframe/properties/configpropertywidget.cpp
    cmake/project/mainframe/properties/configutil.cpp
    cmake/project/mainframe/cbp/cbpparser.cpp
    cmake/project/transceiver/projectcmakereceiver.cpp
    cmake/option/kitsmanagerwidget.cpp
    cmake/option/optioncmakegenerator.cpp
    cmake/option/cmakeoptionwidget.cpp
    cmake/option/kitmanager.cpp
    cmake/option/kit.cpp
    cmake/option/persistentsettings.cpp
    ninja/builder/mainframe/ninjabuildergenerator.cpp
    ninja/builder/parser/ninjaparser.cpp
    ninja/project/ninjaasynparse.cpp
    ninja/project/ninjaprojectgenerator.cpp
    ninja/option/ninjawidget.cpp
    ninja/option/ninjaoptionwidget.cpp
    ninja/option/optionninjagenerator.cpp
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
    cmake/builder/mainframe/cmakebuildergenerator.h
    cmake/builder/parser/cmakeparser.h
    cmake/builder/parser/ansifilterparser.h
    cmake/builder/parser/gnumakeparser.h
    cmake/builder/parser/gccparser.h
    cmake/builder/parser/ldparser.h
    cmake/project/mainframe/cmakeasynparse.h
    cmake/project/mainframe/cmakeitemkeeper.h
    cmake/project/mainframe/cmakeprojectgenerator.h
    cmake/project/mainframe/properties/environmentwidget.h
    cmake/project/mainframe/properties/stepspane.h
    cmake/project/mainframe/properties/targetsmanager.h
    cmake/project/mainframe/properties/runconfigpane.h
    cmake/project/mainframe/properties/configureprojpane.h
    cmake/project/mainframe/properties/buildpropertywidget.h
    cmake/project/mainframe/properties/runpropertywidget.h
    cmake/project/mainframe/properties/configpropertywidget.h
    cmake/project/mainframe/properties/configutil.h
    cmake/project/mainframe/cbp/cbpparser.h
    cmake/project/transceiver/projectcmakereceiver.h
    cmake/option/kitsmanagerwidget.h
    cmake/option/optioncmakegenerator.h
    cmake/option/cmakeoptionwidget.h
    cmake/option/kitmanager.h
    cmake/option/kit.h
    cmake/option/persistentsettings.h
    ninja/builder/mainframe/ninjabuildergenerator.h
    ninja/builder/parser/ninjaparser.h
    ninja/project/ninjaasynparse.h
    ninja/project/ninjaprojectgenerator.h
    ninja/option/ninjawidget.h
    ninja/option/ninjaoptionwidget.h
    ninja/option/optionninjagenerator.h
    cxxplugin.h
    )

set(CXX_QRC
    cmake/project/mainframe/cmake.qrc
    ninja/project/ninja.qrc
    )

set(QT_THEME)

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    ${CXX_QRC}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


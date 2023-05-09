cmake_minimum_required(VERSION 3.0.2)

project(plugin-projectcmake)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    mainframe/cmakeasynparse.cpp
    mainframe/cmakeitemkeeper.cpp
    mainframe/cmakegenerator.cpp
    mainframe/properties/environmentwidget.cpp
    mainframe/properties/stepspane.cpp
    mainframe/properties/targetsmanager.cpp
    mainframe/properties/runconfigpane.cpp
    mainframe/properties/configureprojpane.cpp
    mainframe/properties/buildpropertywidget.cpp
    mainframe/properties/runpropertywidget.cpp
    mainframe/properties/configpropertywidget.cpp
    mainframe/properties/configutil.cpp
    mainframe/cbp/cbpparser.cpp
    transceiver/projectcmakereceiver.cpp
    projectcmake.cpp
    projectcmake.json
    )

set(CXX_H
    mainframe/cmakeasynparse.h
    mainframe/cmakeitemkeeper.h
    mainframe/cmakegenerator.h
    mainframe/properties/environmentwidget.h
    mainframe/properties/stepspane.h
    mainframe/properties/targetsmanager.h
    mainframe/properties/runconfigpane.h
    mainframe/properties/configureprojpane.h
    mainframe/properties/buildpropertywidget.h
    mainframe/properties/runpropertywidget.h
    mainframe/properties/configpropertywidget.h
    mainframe/properties/configutil.h
    mainframe/cbp/cbpparser.h
    transceiver/projectcmakereceiver.h
    projectcmake.h
    )

set(CXX_QRC
    mainframe/cmake.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${CXX_QRC}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



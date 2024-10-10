cmake_minimum_required(VERSION 3.0.2)

project(python)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_CPP
    python/pythondebug.cpp
    python/pythongenerator.cpp
    python/project/pythonprojectgenerator.cpp
    python/project/properties/configpropertywidget.cpp
    python/project/properties/configutil.cpp
    python/option/interpreterwidget.cpp
    python/option/pythonoptionwidget.cpp
    python/option/optionpythongenerator.cpp
    lexer/scilexerpython.cpp
    pythonplugin.cpp
    pythonplugin.json
    )

set(CXX_H
    python/pythondebug.h
    python/pythongenerator.h
    python/project/pythonprojectgenerator.h
    python/project/properties/configpropertywidget.h
    python/project/properties/configutil.h
    python/option/interpreterwidget.h
    python/option/pythonoptionwidget.h
    python/option/optionpythongenerator.h
    lexer/scilexerpython.h
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
    duc-framework
    duc-base
    duc-services
    duc-common
    cppdap
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


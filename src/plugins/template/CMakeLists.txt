cmake_minimum_required(VERSION 3.0.2)

project(plugin-template)

set(CXX_CPP
    wizard/detailwidget.cpp
    wizard/maindialog.cpp
    wizard/templateparser.cpp
    wizard/projectgenerate.cpp
    templatemanager.cpp
    templateplugin.cpp
    templateplugin.json
    )

set(CXX_H
    wizard/detailwidget.h
    wizard/maindialog.h
    wizard/templateparser.h
    wizard/projectgenerate.h
    templatemanager.h
    templateplugin.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    template.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


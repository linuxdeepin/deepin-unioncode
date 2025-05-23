cmake_minimum_required(VERSION 3.0.2)

project(template)

set(CXX_CPP
    wizard/detailwidget.cpp
    wizard/maindialog.cpp
    wizard/templateparser.cpp
    wizard/projectgenerate.cpp
    wizard/projectpane.cpp
    wizard/fieldspane.cpp
    wizard/kitspane.cpp
    transceiver/templatereceiver.cpp
    templatemanager.cpp
    templateplugin.cpp
    templateplugin.json
    )

set(CXX_H
    wizard/detailwidget.h
    wizard/maindialog.h
    wizard/templateparser.h
    wizard/projectgenerate.h
    wizard/projectpane.h
    wizard/fieldspane.h
    wizard/abstractpane.h
    wizard/kitspane.h
    transceiver/templatereceiver.h
    templatemanager.h
    templateplugin.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    duc-common
    ${QtUseModules}
    ${PkgUserModules}
    ${DtkWidget_LIBRARIES}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})


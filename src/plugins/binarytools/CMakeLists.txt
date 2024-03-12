cmake_minimum_required(VERSION 3.0.2)

project(binarytools)

set(CXX_CPP
    mainframe/binarytoolsconfigview.cpp
    mainframe/binarytoolsdialog.cpp
    mainframe/binarytoolssetting.cpp
    mainframe/environmentview.cpp
    binarytools.cpp
    binarytools.json
    )

set(CXX_H
    mainframe/binarytoolsconfigview.h
    mainframe/binarytoolsdialog.h
    mainframe/binarytoolssetting.h
    mainframe/environmentview.h
    binarytools.h
    )


add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    binarytools.qrc
    )

target_link_libraries(${PROJECT_NAME}
    framework
    common
    base
    services
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



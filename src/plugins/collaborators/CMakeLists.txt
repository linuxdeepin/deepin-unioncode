cmake_minimum_required(VERSION 3.0.2)

project(collaborators)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient.cmake)

set(CXX_CPP
    transceiver/collaboratorsreceiver.cpp
    mainframe/git/gitqlientwidget.cpp
    mainframe/svn/checkoutdialog.cpp
    mainframe/svn/historydiffview.cpp
    mainframe/svn/historydiffwidget.cpp
    mainframe/svn/historylogwidget.cpp
    mainframe/svn/historydisplaywidget.cpp
    mainframe/svn/historyview.cpp
    mainframe/svn/filesourcemodel.cpp
    mainframe/svn/filesourcedelegate.cpp
    mainframe/svn/filesourceview.cpp
    mainframe/svn/loggindialog.cpp
    mainframe/svn/svnclientwidget.cpp
    mainframe/svn/reposwidget.cpp
    mainframe/svn/amendswidget.cpp
    mainframe/svn/filemodifyview.cpp
    mainframe/cvskeeper.cpp
    collaborators.cpp
    collaborators.json
    )

set(CXX_H
    transceiver/collaboratorsreceiver.h
    mainframe/git/gitqlientwidget.h
    mainframe/svn/checkoutdialog.h
    mainframe/svn/historydiffview.h
    mainframe/svn/historydiffwidget.h
    mainframe/svn/historylogwidget.h
    mainframe/svn/historydisplaywidget.h
    mainframe/svn/historyview.h
    mainframe/svn/filesourcemodel.h
    mainframe/svn/filesourcedelegate.h
    mainframe/svn/filesourceview.h
    mainframe/svn/loggindialog.h
    mainframe/svn/svnclientwidget.h
    mainframe/svn/reposwidget.h
    mainframe/svn/basetype.h
    mainframe/svn/amendswidget.h
    mainframe/svn/filemodifyview.h
    mainframe/cvskeeper.h
    collaborators.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    collaborators.qrc
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



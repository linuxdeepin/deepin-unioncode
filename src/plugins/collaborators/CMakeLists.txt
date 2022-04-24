cmake_minimum_required(VERSION 3.0.2)

project(plugin-collaborators)

include(${CMAKE_SOURCE_DIR}/3rdparty/GitQlient.cmake)

set(CXX_CPP
    mainframe/git/gitqlientwidget.cpp
    mainframe/svn/loggindialog.cpp
    mainframe/svn/svnclientwidget.cpp
    mainframe/svn/reposwidget.cpp
    mainframe/svn/commithistorywidget.cpp
    mainframe/svn/commithistorymodel.cpp
    mainframe/svn/commithistoryview.cpp
    mainframe/svn/amendswidget.cpp
    mainframe/svn/filemodifywidget.cpp
    collaborators.cpp
    collaborators.json
    )

set(CXX_H
    mainframe/git/gitqlientwidget.h
    mainframe/svn/loggindialog.h
    mainframe/svn/svnclientwidget.h
    mainframe/svn/reposwidget.h
    mainframe/svn/basetype.h
    mainframe/svn/commithistorywidget.h
    mainframe/svn/commithistorymodel.h
    mainframe/svn/commithistoryview.h
    mainframe/svn/amendswidget.h
    mainframe/svn/filemodifywidget.h
    collaborators.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    unioncode-framework
    unioncode-base
    unioncode-service
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})



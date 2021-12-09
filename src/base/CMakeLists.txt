project(unioncode-base)

set(CXX_H
    abstractcentral.h
    abstractedit.h
    abstractmenu.h
    abstractmenubar.h
    abstractnav.h
    abstractaction.h
    abstractmainwindow.h
    )

set(CXX_CPP
    abstractcentral.cpp
    abstractedit.cpp
    abstractmenu.cpp
    abstractmenubar.cpp
    abstractnav.cpp
    abstractaction.cpp
    abstractmainwindow.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

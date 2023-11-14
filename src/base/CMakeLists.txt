project(base)

set(CXX_H
    abstractcentral.h
    abstractedit.h
    abstractmenu.h
    abstractmenubar.h
    abstractnav.h
    abstractaction.h
    abstractmainwindow.h
    abstractwidget.h
    abstractoutputpane.h
    abstractconsole.h
    abstractdebugger.h
    )

set(CXX_CPP
    abstractcentral.cpp
    abstractedit.cpp
    abstractmenu.cpp
    abstractmenubar.cpp
    abstractnav.cpp
    abstractaction.cpp
    abstractmainwindow.cpp
    abstractwidget.cpp
    abstractoutputpane.cpp
    abstractconsole.cpp
    abstractdebugger.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${DtkWidget_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})

project(base)

set(CXX_H
    abstractmenu.h
    abstractaction.h
    abstractwidget.h
    abstractdebugger.h
    abstractlexerproxy.h
    )

set(CXX_CPP
    abstractmenu.cpp
    abstractaction.cpp
    abstractwidget.cpp
    abstractdebugger.cpp
    abstractlexerproxy.cpp
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


project(unioncode-service)

set(CXX_H
    window/windowservice.h
    debugger/debuggerservice.h
    )

set(CXX_CPP
    treeview/
    window/windowservice.cpp
    debugger/debuggerservice.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

link_libraries(
    ${PROJECT_NAME}
    unioncode-base
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

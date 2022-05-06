
project(unioncode-service)

set(CXX_H
    window/windowservice.h
    debugger/debuggerservice.h
    workspace/workspaceservice.h
    container/containerservice.h
    project/projectgenerator.h
    project/projectservice.h
    )

set(CXX_CPP
    window/windowservice.cpp
    debugger/debuggerservice.cpp
    workspace/workspaceservice.cpp
    project/projectgenerator.cpp
    container/containerservice.cpp
    project/projectservice.cpp
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

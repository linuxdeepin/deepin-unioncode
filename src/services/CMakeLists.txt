
project(unioncode-service)

set(CXX_H
    window/windowservice.h
    debugger/debuggerservice.h
    container/containerservice.h
    project/projectinfo.h
    project/projectgenerator.h
    project/projectservice.h
    runtime/runtimeservice.h
    )

set(CXX_CPP
    window/windowservice.cpp
    debugger/debuggerservice.cpp
    container/containerservice.cpp
    project/projectservice.cpp
    runtime/runtimeservice.cpp
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

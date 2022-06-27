
project(unioncode-service)

set(CXX_H
    window/windowelement.h
    window/windowservice.h
    debugger/debuggerservice.h
    container/containerservice.h
    project/symbolinfo.h
    project/symbolgenerator.h
    project/symbolviewinterface.h
    project/projectviewinterface.h
    project/projectinfo.h
    project/projectgenerator.h
    project/projectservice.h
    runtime/runtimeservice.h
    find/enums.h
    find/findservice.h
    option/optionservice.h
    option/optiongenerator.h
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    )

link_libraries(
    ${PROJECT_NAME}
    unioncode-base
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

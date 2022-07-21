
project(unioncode-service)

set(CXX_H
    window/windowelement.h
    window/windowservice.h
    debugger/debuggerservice.h
    container/containerservice.h
    project/projectactioninfo.h
    project/symbolinfo.h
    project/symbolgenerator.h
    project/symbolviewinterface.h
    project/projectviewinterface.h
    project/projectinfo.h
    project/projectgenerator.h
    project/projectservice.h
    find/enums.h
    find/findservice.h
    option/optionservice.h
    option/optiongenerator.h
    option/toolchaindata.h
    option/toolchaindata.cpp
    option/optionutils.cpp
    option/optionutils.h
    option/optionmanager.h
    option/optionmanager.cpp
    option/optiondatastruct.h
    builder/builderservice.h
    builder/buildergenerator.h
    builder/builderglobals.h
    builder/builderinterface.h
    builder/task.h
    builder/task.cpp
    builder/fileutils.h
    builder/fileutils.cpp
    builder/ioutputparser.h
    builder/ioutputparser.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    builder/resources.qrc
    )

link_libraries(
    ${PROJECT_NAME}
    unioncode-base
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

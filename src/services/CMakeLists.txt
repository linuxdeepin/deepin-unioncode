project(services)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXX_H
    toolchecker/toolcheckerservice.h
    window/windowelement.h
    window/windowservice.h
    debugger/debuggerservice.h
    container/containerservice.h
    project/projectactioninfo.h
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
    language/languagegenerator.h
    language/languageservice.h
    symbol/symbolservice.h
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
    base
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

target_link_libraries(${PROJECT_NAME}
    cppdap
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

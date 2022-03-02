cmake_minimum_required(VERSION 3.0.2)

project(cxxdbg)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXXDBG_HEADERS
    debugmanager.h
    gdbproxy.h
    debugengine.h
    dapsession.h
    backendglobal.h
   )

set(CXXDBG_SOURCES
    debugmanager.cpp
    gdbproxy.cpp
    main.cpp
    debugengine.cpp
    dapsession.cpp
    )

link_libraries(
    cppdap
    pthread
    )

add_executable(cxxdbg main.cpp
    ${CXXDBG_HEADERS}
    ${CXXDBG_SOURCES})
#add_executable(cxxdbg ${CXXDBG_SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")

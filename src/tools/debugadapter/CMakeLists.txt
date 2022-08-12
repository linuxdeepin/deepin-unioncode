cmake_minimum_required(VERSION 3.0.2)

project(debugadapter)

#include(${CMAKE_SOURCE_DIR}/3rdparty/cppdap.cmake)

set(${PROJECT_NAME}_HEADERS
    debugmanager.h
    dapproxy.h
    debugengine.h
    dapsession.h
    backendglobal.h
    locker.h
    serverinfo.h
    debugger/debugger.h
    debugger/gdbmi/gdbmi.h
    debugger/gdbmi/gdbdebugger.h
   )

set(${PROJECT_NAME}_SOURCES
    debugmanager.cpp
    dapproxy.cpp
    main.cpp
    debugengine.cpp
    dapsession.cpp
    serverinfo.cpp
    debugger/debugger.cpp
    debugger/gdbmi/gdbmi.cpp
    debugger/gdbmi/gdbdebugger.cpp
    )

link_libraries(
    cppdap
    pthread
    )

add_executable(${PROJECT_NAME} main.cpp
    ${${PROJECT_NAME}_HEADERS}
    ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
  Qt5::DBus
  )

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")

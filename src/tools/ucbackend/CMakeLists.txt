cmake_minimum_required(VERSION 3.0.2)

project(ucbackend)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)
find_package(Qt5 CONFIG REQUIRED Core DBus)

set(UCBACKEND_HEADERS
    debugmanager.h
    gdbproxy.h
    debugengine.h
    dapsession.h
    backendglobal.h
    locker.h
    serverinfo.h
   )

set(UCBACKEND_SOURCES
    debugmanager.cpp
    gdbproxy.cpp
    main.cpp
    debugengine.cpp
    dapsession.cpp
    serverinfo.cpp
    )

link_libraries(
    cppdap
    pthread
    )

add_executable(ucbackend main.cpp
    ${UCBACKEND_HEADERS}
    ${UCBACKEND_SOURCES})

target_link_libraries(ucbackend
  Qt5::DBus
  )

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")

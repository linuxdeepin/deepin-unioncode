cmake_minimum_required(VERSION 3.0.2)

project(cxxdbg)

set(CPPDAP_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cppdap)
include_directories(${CPPDAP_DIR}/include)

set(CXXDBG_HEADERS
    server.h
    protocol.h
    context.h
    cxxdebugger.h
   )
set(CXXDBG_SOURCES
    server.cpp
    context.cpp
    cxxdebugger.cpp
    main.cpp
    )

link_libraries(
    cppdap
    pthread
    )

add_executable(cxxdbg main.cpp ${CXXDBG_SOURCES})
#add_executable(cxxdbg ${CXXDBG_SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_INSTALL_PATH})

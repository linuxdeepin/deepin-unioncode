project(unioncode-jsonrpccpp)

set(JSONRPCCPP_ROOT "libjson-rpc-cpp") #1.4.1
############### jsonrpccpp ###############

# setup compiler settings && dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${JSONRPCCPP_ROOT}/cmake")
message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

include(CMakeCompilerSettings)
include(CMakeDependencies)

option(HUNTER_ENABLED "Enable Hunter package manager" NO)
include(HunterGate)
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.18.47.tar.gz"
        SHA1 "f0c476a3a0f9edd6182936d8c4460811f112189c"
)

if(NOT MSVC)
    set(BUILD_SHARED_LIBS YES CACHE BOOL "Build shared libraries")
    set(BUILD_STATIC_LIBS NO CACHE BOOL "Build static libraries")
else()
    set(BUILD_SHARED_LIBS NO CACHE BOOL "Build shared libraries")
    set(BUILD_STATIC_LIBS YES CACHE BOOL "Build static libraries")
endif()
set(LIB_SUFFIX "" CACHE STRING "Suffix for library directory (32/64)")

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "Both BUILD_SHARED_LIBS and BUILD_STATIC_LIBS are disabled")
endif()

# defaults for modules that can be enabled/disabled
if(UNIX)
    set(UNIX_DOMAIN_SOCKET_SERVER NO CACHE BOOL "Include Unix Domain Socket server")
    set(UNIX_DOMAIN_SOCKET_CLIENT NO CACHE BOOL "Include Unix Domain Socket client")
    set(FILE_DESCRIPTOR_SERVER NO CACHE BOOL "Include File Descriptor server")
    set(FILE_DESCRIPTOR_CLIENT NO CACHE BOOL "Include File Descriptor client")
endif(UNIX)

set(TCP_SOCKET_SERVER YES CACHE BOOL "Include Tcp Socket server")
set(TCP_SOCKET_CLIENT YES CACHE BOOL "Include Tcp Socket client")
set(SERIAL_PORT_SERVER NO CACHE BOOL "Include Serial port server")
set(SERIAL_PORT_CLIENT NO CACHE BOOL "Include Serial port client")
set(REDIS_SERVER NO CACHE BOOL "Include Redis server using hiredis")
set(REDIS_CLIENT NO CACHE BOOL "Include Redis client using hiredis")
set(HTTP_SERVER NO CACHE BOOL "Include HTTP server using libmicrohttpd")
set(HTTP_CLIENT NO CACHE BOOL "Include HTTP client support using curl")
set(COMPILE_TESTS NO CACHE BOOL "Compile test framework")
set(COMPILE_STUBGEN NO CACHE BOOL "Compile the stubgenerator")
set(COMPILE_EXAMPLES NO CACHE BOOL "Compile example programs")

option(WITH_COVERAGE "Build with code coverage flags" OFF)

# print actual settings
if(UNIX)
    message(STATUS "UNIX_DOMAIN_SOCKET_SERVER: ${UNIX_DOMAIN_SOCKET_SERVER}")
    message(STATUS "UNIX_DOMAIN_SOCKET_CLIENT: ${UNIX_DOMAIN_SOCKET_CLIENT}")
endif(UNIX)
message(STATUS "TCP_SOCKET_SERVER: ${TCP_SOCKET_SERVER}")
message(STATUS "TCP_SOCKET_CLIENT: ${TCP_SOCKET_CLIENT}")
message(STATUS "HTTP_SERVER: ${HTTP_SERVER}")
message(STATUS "HTTP_CLIENT: ${HTTP_CLIENT}")
message(STATUS "REDIS_SERVER: ${REDIS_SERVER}")
message(STATUS "REDIS_CLIENT: ${REDIS_CLIENT}")
if(UNIX)
    message(STATUS "UNIXDOMAINSOCKET_SERVER: ${UNIX_DOMAIN_SOCKET_SERVER}")
    message(STATUS "UNIXDOMAINSOCKET_CLIENT: ${UNIX_DOMAIN_SOCKET_CLIENT}")
endif(UNIX)
message(STATUS "COMPILE_TESTS: ${COMPILE_TESTS}")
message(STATUS "COMPILE_STUBGEN: ${COMPILE_STUBGEN}")
message(STATUS "COMPILE_EXAMPLES: ${COMPILE_EXAMPLES}")

# setup common headers and sources
file(GLOB jsonrpc_header ${JSONRPCCPP_ROOT}/*.h)
file(GLOB jsonrpc_header_common ${JSONRPCCPP_ROOT}/src/jsonrpccpp/common/*.h)
file(GLOB jsonrpc_source_common ${JSONRPCCPP_ROOT}/src/jsonrpccpp/common/*.c*)

file(GLOB jsonrpc_header_server ${JSONRPCCPP_ROOT}/src/jsonrpccpp/server/*.h)
file(GLOB jsonrpc_source_server ${JSONRPCCPP_ROOT}/src/jsonrpccpp/server/*.c*)

file(GLOB jsonrpc_header_client ${JSONRPCCPP_ROOT}/src/jsonrpccpp/client/*.h)
file(GLOB jsonrpc_source_client ${JSONRPCCPP_ROOT}/src/jsonrpccpp/client/*.c*)

# setup connector variables defaults
set(client_connector_source "")
set(client_connector_header "")
set(client_connector_libs "")
set(server_connector_source "")
set(server_connector_header "")
set(server_connector_libs "")
set(SERVER_LIBS "")
set(CLIENT_LIBS "")

# setup sources for http connectors
set(CLIENT_CONNECTORS ${JSONRPCCPP_ROOT}/src/jsonrpccpp/client/connectors)
set(SERVER_CONNECTORS ${JSONRPCCPP_ROOT}/src/jsonrpccpp/server/connectors)

if (HTTP_CLIENT)
    list(APPEND client_connector_header "${CLIENT_CONNECTORS}/httpclient.h")
    list(APPEND client_connector_source "${CLIENT_CONNECTORS}/httpclient.cpp")
    list(APPEND client_connector_libs ${CURL_LIBRARIES})
    set(CLIENT_LIBS "${CLIENT_LIBS} -lcurl")
endif ()

if (HTTP_SERVER)
    list(APPEND server_connector_header "${SERVER_CONNECTORS}/httpserver.h")
    list(APPEND server_connector_source "${SERVER_CONNECTORS}/httpserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT} ${MHD_LIBRARIES})
    set(SERVER_LIBS "${SERVER_LIBS} -lmicrohttpd")
endif ()

# setup sources for redis connectors
if (REDIS_CLIENT)
    list(APPEND client_connector_header "${CLIENT_CONNECTORS}/redisclient.h")
    list(APPEND client_connector_source "${CLIENT_CONNECTORS}/redisclient.cpp")
    list(APPEND client_connector_libs ${HIREDIS_LIBRARIES})
    include_directories(${HIREDIS_INCLUDE_DIRS})
    set(CLIENT_LIBS "${CLIENT_LIBS} -lhiredis")
endif ()

if (REDIS_SERVER)
    list(APPEND server_connector_header "${SERVER_CONNECTORS}/redisserver.h")
    list(APPEND server_connector_source "${SERVER_CONNECTORS}/redisserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT} ${HIREDIS_LIBRARIES})
    set(SERVER_LIBS "${SERVER_LIBS} -lhiredis")
endif ()

# setup sources for unix domain socket connectors
if (UNIX_DOMAIN_SOCKET_SERVER)
    list(APPEND server_connector_header "${SERVER_CONNECTORS}/unixdomainsocketserver.h")
    list(APPEND server_connector_source "${SERVER_CONNECTORS}/unixdomainsocketserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (UNIX_DOMAIN_SOCKET_CLIENT)
    list(APPEND client_connector_header "${CLIENT_CONNECTORS}/unixdomainsocketclient.h")
    list(APPEND client_connector_source "${CLIENT_CONNECTORS}/unixdomainsocketclient.cpp")
endif ()

# setup sources for tcp socket connectors
if (TCP_SOCKET_SERVER)
    list(APPEND server_connector_header "${SERVER_CONNECTORS}/tcpsocketserver.h")
    list(APPEND server_connector_source "${SERVER_CONNECTORS}/tcpsocketserver.cpp")
    if (WIN32)
        list(APPEND server_connector_header "${SERVER_CONNECTORS}/windowstcpsocketserver.h")
        list(APPEND server_connector_source "${SERVER_CONNECTORS}/windowstcpsocketserver.cpp")
        list(APPEND server_connector_libs ws2_32)
    endif ()
    if (UNIX)
        list(APPEND server_connector_header "${SERVER_CONNECTORS}/linuxtcpsocketserver.h")
        list(APPEND server_connector_source "${SERVER_CONNECTORS}/linuxtcpsocketserver.cpp")
    endif ()
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (TCP_SOCKET_CLIENT)
    list(APPEND client_connector_header "${CLIENT_CONNECTORS}/tcpsocketclient.h")
    list(APPEND client_connector_source "${CLIENT_CONNECTORS}/tcpsocketclient.cpp")
    if (WIN32)
        list(APPEND client_connector_header "${CLIENT_CONNECTORS}/windowstcpsocketclient.h")
        list(APPEND client_connector_source "${CLIENT_CONNECTORS}/windowstcpsocketclient.cpp")
        list(APPEND client_connector_libs ws2_32)
    endif ()
    if (UNIX)
        list(APPEND client_connector_header "${CLIENT_CONNECTORS}/linuxtcpsocketclient.h")
        list(APPEND client_connector_source "${CLIENT_CONNECTORS}/linuxtcpsocketclient.cpp")
    endif ()
    list(APPEND client_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (SERIAL_PORT_SERVER)
    if (UNIX)
        list(APPEND server_connector_header "${SERVER_CONNECTORS}/linuxserialportserver.h")
        list(APPEND server_connector_source "${SERVER_CONNECTORS}/linuxserialportserver.cpp")
    endif ()
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (SERIAL_PORT_CLIENT)
    if (UNIX)
        list(APPEND client_connector_header "${CLIENT_CONNECTORS}/linuxserialportclient.h")
        list(APPEND client_connector_source "${CLIENT_CONNECTORS}/linuxserialportclient.cpp")
    endif ()
    list(APPEND client_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (FILE_DESCRIPTOR_SERVER)
    list(APPEND server_connector_header "${SERVER_CONNECTORS}/filedescriptorserver.h")
    list(APPEND server_connector_source "${SERVER_CONNECTORS}/filedescriptorserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (FILE_DESCRIPTOR_CLIENT)
    list(APPEND client_connector_header "${CLIENT_CONNECTORS}/filedescriptorclient.h")
    list(APPEND client_connector_source "${CLIENT_CONNECTORS}/filedescriptorclient.cpp")
    list(APPEND client_connector_libs ${CMAKE_THREAD_LIBS_INIT})
endif ()

# configure a header file to pass some of the CMake settings to the source code
file(MAKE_DIRECTORY "${JSONRPCCPP_ROOT}/src/jsonrpccpp/common")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${JSONRPCCPP_ROOT}/src/jsonrpccpp/common/jsonparser.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/${JSONRPCCPP_ROOT}/src/jsonrpccpp/common/jsonparser.h"
    )

# include required directories
include_directories(${JSONRPCCPP_ROOT}/src)
include_directories(${MHD_INCLUDE_DIRS})

# setup shared common library
if (BUILD_SHARED_LIBS)
    add_library(unioncode-jsonrpccommon SHARED ${jsonrpc_source_common} ${jsonrpc_header} ${jsonrpc_helper_source_common})
    target_link_libraries(unioncode-jsonrpccommon ${JSONCPP_LIBRARY})
    set_target_properties(unioncode-jsonrpccommon PROPERTIES OUTPUT_NAME unioncode-jsonrpccpp-common)
endif ()

# setup static common library
if (BUILD_STATIC_LIBS OR MSVC)
    add_library(common STATIC ${jsonrpc_source_common} ${jsonrpc_header} ${jsonrpc_helper_source_common})
    target_link_libraries(common jsoncpp_lib_static)
    set_target_properties(common PROPERTIES OUTPUT_NAME jsonrpccpp-common)

    if (NOT BUILD_SHARED_LIBS)
        add_library(unioncode-jsonrpccommon ALIAS common)
    endif ()
endif ()

# setup shared client library
if (BUILD_SHARED_LIBS)
    add_library(unioncode-jsonrpcclient SHARED ${jsonrpc_source_client} ${jsonrpc_header} ${jsonrpc_header_client} ${client_connector_source})
    add_dependencies(unioncode-jsonrpcclient unioncode-jsonrpccommon)
    target_link_libraries(unioncode-jsonrpcclient unioncode-jsonrpccommon ${client_connector_libs})
    set_target_properties(unioncode-jsonrpcclient PROPERTIES OUTPUT_NAME unioncode-jsonrpccpp-client)
endif ()

# setup static client library
if (BUILD_STATIC_LIBS OR MSVC)
    add_library(client STATIC ${jsonrpc_source_client} ${jsonrpc_header} ${jsonrpc_header_client} ${client_connector_source})
    target_link_libraries(client common ${client_connector_libs})
    set_target_properties(client PROPERTIES OUTPUT_NAME jsonrpccpp-client)

    if (NOT BUILD_SHARED_LIBS)
        add_library(unioncode-jsonrpcclient ALIAS client)
    endif ()
endif ()

# setup shared server library
if (BUILD_SHARED_LIBS)
    add_library(unioncode-jsonrpcserver SHARED ${jsonrpc_source_server} ${jsonrpc_header} ${jsonrpc_header_server} ${server_connector_source})
    add_dependencies(unioncode-jsonrpcserver unioncode-jsonrpccommon)
    target_link_libraries(unioncode-jsonrpcserver unioncode-jsonrpccommon ${server_connector_libs})
    set_target_properties(unioncode-jsonrpcserver PROPERTIES OUTPUT_NAME unioncode-jsonrpccpp-server)
endif ()

# setup static server library
if (BUILD_STATIC_LIBS OR MSVC)
    add_library(server STATIC ${jsonrpc_source_server} ${jsonrpc_header} ${jsonrpc_header_server} ${server_connector_source})
    target_link_libraries(server common ${server_connector_libs})
    set_target_properties(server PROPERTIES OUTPUT_NAME jsonrpccpp-server)

    if (NOT BUILD_SHARED_LIBS)
        add_library(jsonrpcserver ALIAS server)
    endif ()
endif ()

set(ALL_LIBS)

if (BUILD_SHARED_LIBS OR NOT BUILD_STATIC_LIBS)
    list(APPEND ALL_LIBS unioncode-jsonrpccommon unioncode-jsonrpcclient unioncode-jsonrpcserver)
endif ()

if (BUILD_STATIC_LIBS OR MSVC)
    list(APPEND ALL_LIBS common client server)
endif ()

install(TARGETS unioncode-jsonrpccommon LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})
install(TARGETS unioncode-jsonrpcserver LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})
install(TARGETS unioncode-jsonrpcclient LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})


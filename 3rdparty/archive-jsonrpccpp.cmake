
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/archive-jsonrpccpp/jsoncpp/include/json")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/archive-jsonrpccpp/libjson-rpc-cpp/src")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/archive-jsonrpccpp/libjson-rpc-cpp/src/jsonrpccpp/common")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdparty/archive-jsonrpccpp/libjson-rpc-cpp/cmake")

include(CMakeDependencies)

include_directories("${JSONCPP_INCLUDE_DIR}")

link_libraries(
        ${JSONCPP_LIBRARY}
        ${ARGTABLE_LIBRARIES}
        ${CURL_LIBRARIES}
        ${MHD_LIBRARIES}
        ${HIREDIS_LIBRARIES}
)

# targets
#archive-jsonrpccpp-client
#archive-jsonrpccpp-server
#archive-jsonrpccpp-common
#archive-jsoncpp
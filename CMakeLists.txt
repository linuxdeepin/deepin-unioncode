cmake_minimum_required(VERSION 3.18.0)

project(deepin-unioncode)

message("processor: ${CMAKE_SYSTEM_PROCESSOR}")

function(print_directory arg)
    execute_process(COMMAND ls ${ARGV0} OUTPUT_VARIABLE outVar)
    message(STATUS "print_directory native: ${ARGV0}\n" ${outVar})
endfunction()

set(VERSION $ENV{VERSION})

if (NOT VERSION)
    set(VERSION "0.0.1")
endif()

message(STATUS "build version ${VERSION}")
option(ENABLE_SANITIZE_CHECK "Enable Sanitize Check " OFF) 

set(CMAKE_VERBOSE_MAKEFILE ON)

if (ENABLE_SANITIZE_CHECK AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
#    set(CMAKE_C_FLAGS "-fsanitize=thread")
#    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

set(LIBRARY_INSTALL_PREFIX "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${PROJECT_NAME}")

#set(ARCHIVE_INSTALL_RPEFIX "") undefined the static lib archive
set(RUNTIME_INSTALL_RPEFIX "/usr/bin")
set(SOURCES_INSTALL_RPEFIX "/usr/share/${PROJECT_NAME}")
set(INCLUDE_INSTALL_RPEFIX "/usr/include/${PROJECT_NAME}")

set(PLUGIN_INSTALL_PATH ${LIBRARY_INSTALL_PREFIX}/plugins)
set(TOOLS_INSTALL_PATH ${LIBRARY_INSTALL_PREFIX}/tools)
# set install runtime path
set(CMAKE_INSTALL_RPATH ${LIBRARY_INSTALL_PREFIX} ${PLUGIN_PATH})

message(STATUS "LIBRARY_INSTALL_PREFIX ${LIBRARY_INSTALL_PREFIX}")
message(STATUS "RUNTIME_INSTALL_RPEFIX ${RUNTIME_INSTALL_RPEFIX}")
message(STATUS "SOURCES_INSTALL_RPEFIX ${SOURCES_INSTALL_RPEFIX}")
message(STATUS "INCLUDE_INSTALL_RPEFIX ${INCLUDE_INSTALL_RPEFIX}")
message(STATUS "PLUGIN_INSTALL_PATH ${PLUGIN_INSTALL_PATH}")
message(STATUS "TOOLS_INSTALL_PATH ${TOOLS_INSTALL_PATH}")
message(STATUS "CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}")

# build runtime path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_definitions(-DQT_MESSAGELOGCONTEXT)

# Setup the environment
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if ((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "sw_64")
        OR (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
        OR (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
        )
    message(STATUS "add cmake params found thread library from ${CMAKE_SYSTEM_PROCESSOR}")
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY ON)
    set(CMAKE_USE_WIN32_THREADS_INIT OFF)
    set(CMAKE_USE_PTHREADS_INIT ON)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
set(QT_VERSION_MAJOR 6)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
if (QT_VERSION_MAJOR MATCHES 6)
    set(DTK_VERSION_MAJOR 6)
    set(KF_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
    set(KF_VERSION_MAJOR 5)

endif()
message("   >>> Build with DTK: ${DTK_VERSION_MAJOR}")

find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
include_directories(${PROJECT_NAME} PRIVATE ${DtkWidget_INCLUDE_DIRS})

# Use Qt modules no WebEngineWidgets WebChannel
set(QtFindModules Core Gui Widgets Concurrent Network DBus)

foreach(QtModule ${QtFindModules})
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QtModule} REQUIRED)
    # include qt module private include directors
    include_directories(${Qt${QT_VERSION_MAJOR}${QtModule}_PRIVATE_INCLUDE_DIRS})
    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt${QT_VERSION_MAJOR}::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

set(PkgConfigFindModules)
include(FindPkgConfig)
foreach(PkgConfigModule ${PkgConfigFindModules})
    pkg_check_modules(PkgUseModuleVal REQUIRED ${PkgConfigModule})
    list(APPEND PkgUseModules ${PkgUseModuleVal_LDFLAGS})
    include_directories(${PkgUseModuleVal_INCLUDE_DIRS})
endforeach(PkgConfigModule)

include_directories(${PROJECT_SOURCE_DIR}/src)

option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# sub directories
add_subdirectory(3rdparty)
add_subdirectory(src)
add_subdirectory(assets)

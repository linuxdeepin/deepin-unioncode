cmake_minimum_required(VERSION 3.1.0)

project(unioncode)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    message("debug type open sanitize check")
#    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_VERBOSE_MAKEFILE ON)
#    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
#    set(CMAKE_C_FLAGS "-fsanitize=thread")
#    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

# Setup install paths
#   /usr/bin/dde-file-manager
#       execute bin file
#   /usr/lib/x86_64-linux-gnu/dde-file-manager
#       dir plugins && dir tools abv thumbnail
#   /usr/include/dde-file-manager
#       public *.h && private dir *.h
#   /usr/share/dde-file-manager
#       subdir"database  mimetypeassociations  mimetypes  templates  translations"
set(LIBRARY_INSTALL_PREFIX "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${PROJECT_NAME}")
#set(ARCHIVE_INSTALL_RPEFIX "") undefined the static lib archive
set(RUNTIME_INSTALL_RPEFIX "/usr/bin")
set(SOURCES_INSTALL_RPEFIX "/usr/share/${PROJECT_NAME}")
set(INCLUDE_INSTALL_RPEFIX "/usr/include/${PROJECT_NAME}")

set(PLUGIN_INSTALL_PATH ${LIBRARY_INSTALL_PREFIX}/plugins)

# set install runtime path
set(CMAKE_INSTALL_RPATH ${LIBRARY_INSTALL_PREFIX} ${PLUGIN_PATH})

# build runtime path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})

# if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

# Setup the environment
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# Use Qt modules
set(QtFindModules Core Gui Widgets Concurrent Network)
foreach(QtModule ${QtFindModules})
    find_package(Qt5 COMPONENTS ${QtModule} REQUIRED)
    # include qt module private include directors
    include_directories(${Qt5${QtModule}_PRIVATE_INCLUDE_DIRS})
    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt5::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

# Use PkgConfig modules
# You can setting mini version for "glib-2.0>=2.10 gtk+-2.0" in the list
set(PkgConfigFindModules)
include(FindPkgConfig)
foreach(PkgConfigModule ${PkgConfigFindModules})
    pkg_check_modules(PkgUseModuleVal REQUIRED ${PkgConfigModule})
    #can use target_link_libraries(xxx ${PkgUseModule})
    list(APPEND PkgUseModules ${PkgUseModuleVal_LDFLAGS})
    include_directories(${PkgUseModuleVal_INCLUDE_DIRS})
    #    link_libraries(${PkgUseModuleVal_LIBRARIES})
    #    link_directories(${PkgUseModuleVal_LINK_LIBRARIES})
endforeach(PkgConfigModule)

include_directories(${PROJECT_SOURCE_DIR}/src)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# sub directories
add_subdirectory(3rdparty)
add_subdirectory(src)

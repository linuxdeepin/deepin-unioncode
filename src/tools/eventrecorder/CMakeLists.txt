cmake_minimum_required (VERSION 2.8)
project (emd C CXX ASM)
set (strace2_VERSION_MAJOR 1)
set (strace2_VERSION_MINOR 0)
set (CMAKE_SKIP_BUILD_RPATH true)

# cmake . -DDBG=ON to build debug version
OPTION(DBG
  "Build the project using debugging code"
  OFF)
IF(DBG)
  MESSAGE("Adding Debug flag...")
  ADD_DEFINITIONS(-D_DEBUG)
  SET(CMAKE_BUILD_TYPE Debug)
ELSE()
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(DBG)
MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})

# detect current cpu arch
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )
if( ${ARCHITECTURE} STREQUAL "x86_64" )
    set(FLAGS_COMMON "-msse2 -D__MMX__ -D__SSE__ -D__SSE2__ -D__USE_LARGEFILE64 -pthread -g3")
    set(LIBUNWIND "unwind-x86_64")
    set(LIBUNWIND_PTRACE "unwind-ptrace")
    set(CAPSTONE "capstone")
elseif( ${ARCHITECTURE} STREQUAL "mips64" )
    set(FLAGS_COMMON "-D__MMX__ -D__SSE__ -D__SSE2__ -D__USE_LARGEFILE64 -pthread -g3")
    set(LIBUNWIND "")
    set(LIBUNWIND_PTRACE "")
    set(CAPSTONE "capstone")
elseif( ${ARCHITECTURE} STREQUAL "sw_64" )
    set(FLAGS_COMMON "-D__MMX__ -D__SSE__ -D__SSE2__ -D__USE_LARGEFILE64 -pthread -g3")
    set(LIBUNWIND "")
    set(LIBUNWIND_PTRACE "")
    set(CAPSTONE "")
elseif( ${ARCHITECTURE} STREQUAL "aarch64" )
    set(FLAGS_COMMON "-D__MMX__ -D__SSE__ -D__SSE2__ -D__USE_LARGEFILE64 -pthread -g3")
    set(LIBUNWIND "")
    set(LIBUNWIND_PTRACE "")
    set(CAPSTONE "")
else()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_COMMON} -Wstrict-prototypes -std=gnu11")
# Define __STDC_LIMIT_MACROS so |#include <stdint.h>| works as expected.
# Define __STDC_FORMAT_MACROS so |#include <inttypes.h>| works as expected.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON} -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -std=c++14")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g3")

set(RR_FLAGS_DEBUG "-Wall -Wextra -O0 -DDEBUG -UNDEBUG")
#set(RR_FLAGS_RELEASE "-Wall -Wextra -Werror -O2 -UDEBUG -DNDEBUG")
set(RR_FLAGS_RELEASE "-Wall -Wextra -O2 -UDEBUG -DNDEBUG") #mozart: remove warning as error flag

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(RR_FLAGS ${RR_FLAGS_DEBUG})
else()
  set(RR_FLAGS ${RR_FLAGS_RELEASE})
endif()
MESSAGE("emd build option is :" ${RR_FLAGS})

set(RR_SOURCES
    config.cc
    easylogging++.cc
    trace_writer.cc
    strace2.cc
    utils.cc
    WaitStatus.cc
    zstd_writer.cc
    session.cc
    linenoise.c
    replay.cc
    elf_helper.cc
    debug.cc
)
set_source_files_properties(${RR_SOURCES}
                            PROPERTIES COMPILE_FLAGS ${RR_FLAGS})

add_executable(emd ${RR_SOURCES})

target_link_libraries(emd
  ${CMAKE_DL_LIBS}
  ${LIBUNWIND}
  ${LIBUNWIND_PTRACE}
  ${CAPSTONE}
  -lm
  dl
  stdc++
  json-c
  zstd
  eventman
  dwarf++
  elf++
  elf
  thread_db
  pthread
)


set(PRELOAD_COMPILE_FLAGS "-fno-stack-protector -fPIC")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(PRELOAD_COMPILE_FLAGS "${PRELOAD_COMPILE_FLAGS} ${RR_FLAGS_DEBUG}")
else()
  set(PRELOAD_COMPILE_FLAGS "${PRELOAD_COMPILE_FLAGS} ${RR_FLAGS_RELEASE}")
endif()
MESSAGE("preload build option is :" ${PRELOAD_COMPILE_FLAGS})

# X11 preload
set(X11_PRELOAD_SOURCES
  preload/x11preload.cc
)

set_source_files_properties(${X11_PRELOAD_SOURCES}
                            PROPERTIES COMPILE_FLAGS ${PRELOAD_COMPILE_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")
# We need to know where our generated files are.
# Uncomment this line will always build Makefile in directory same as CMakeLists.txt
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Order matters here! syscall_hook.S must be immediately before syscallbuf.c,
# breakpoint_table.S must be before overrides.c, which must be last.
add_library(x11preload SHARED
  preload/x11preload.cc
)
set_target_properties(x11preload PROPERTIES LINK_FLAGS "-nostartfiles")

target_link_libraries(x11preload
  ${CMAKE_DL_LIBS}
  -ldl
  pthread
  X11
  inprocdump
)

# dbus preload
set(DBUS_PRELOAD_SOURCES
  preload/dbuspreload.cc
)

set_source_files_properties(${DBUS_PRELOAD_SOURCES}
                            PROPERTIES COMPILE_FLAGS ${PRELOAD_COMPILE_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")

if( ${ARCHITECTURE} STREQUAL "x86_64" )
    include_directories("/usr/lib/x86_64-linux-gnu/dbus-1.0/include/")
elseif( ${ARCHITECTURE} STREQUAL "mips64" )
    include_directories("/usr/lib/mips64el-linux-gnuabi64/dbus-1.0/include/"
        "/usr/lib64/dbus-1.0/include/")
elseif( ${ARCHITECTURE} STREQUAL "sw_64" )
    include_directories("/usr/lib/sw_64-linux-gnu/dbus-1.0/include/")
elseif( ${ARCHITECTURE} STREQUAL "aarch64" )
    include_directories("/usr/lib/aarch64-linux-gnu/dbus-1.0/include/")
else()
endif()

include_directories("/usr/include/dbus-1.0/")
# We need to know where our generated files are.
# Uncomment this line will always build Makefile in directory same as CMakeLists.txt
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Order matters here! syscall_hook.S must be immediately before syscallbuf.c,
# breakpoint_table.S must be before overrides.c, which must be last.
add_library(dbuspreload SHARED
  preload/dbuspreload.cc
)
set_target_properties(dbuspreload PROPERTIES LINK_FLAGS "-nostartfiles")

target_link_libraries(dbuspreload
  ${CMAKE_DL_LIBS}
  -ldl
  dbus-1
  inprocdump
)

# vdso preload
set(VDSO_PRELOAD_SOURCES
  preload/vdsopreload.c
)

set_source_files_properties(${VDSO_PRELOAD_SOURCES}
                            PROPERTIES COMPILE_FLAGS ${PRELOAD_COMPILE_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")
# We need to know where our generated files are.
# Uncomment this line will always build Makefile in directory same as CMakeLists.txt
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Order matters here! syscall_hook.S must be immediately before syscallbuf.c,
# breakpoint_table.S must be before overrides.c, which must be last.
add_library(vdsopreload SHARED
  preload/vdsopreload.c
)
set_target_properties(vdsopreload PROPERTIES LINK_FLAGS "-nostartfiles")

target_link_libraries(vdsopreload
  ${CMAKE_DL_LIBS}
  -ldl
)

# inprocdump lib
set(INPROCDUMP_SOURCES
  preload/inprocdump.c
)

set_source_files_properties(${INPROCDUMP_SOURCES}
    PROPERTIES COMPILE_FLAGS ${PRELOAD_COMPILE_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")
# We need to know where our generated files are.
# Uncomment this line will always build Makefile in directory same as CMakeLists.txt
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Order matters here! syscall_hook.S must be immediately before syscallbuf.c,
# breakpoint_table.S must be before overrides.c, which must be last.
add_library(inprocdump SHARED
  preload/inprocdump.c
)
set_target_properties(inprocdump PROPERTIES LINK_FLAGS "-nostartfiles")

target_link_libraries(inprocdump
  ${CMAKE_DL_LIBS}
  -ldl
)

# event manager lib
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(EVENTMAN_COMPILE_FLAGS "${RR_FLAGS_DEBUG}")
else()
    set(EVENTMAN_COMPILE_FLAGS "${RR_FLAGS_RELEASE}")
endif()
MESSAGE("eveentman build option is :" ${EVENTMAN_COMPILE_FLAGS})

set(EVENT_MANAGER_SOURCES
  event_man.cc
  trace_reader.cc
  event.cc
  easylogging++.cc
  config.cc
)

set_source_files_properties(${EVENT_MANAGER_SOURCES}
    PROPERTIES COMPILE_FLAGS ${EVENTMAN_COMPILE_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")
# We need to know where our generated files are.
# Uncomment this line will always build Makefile in directory same as CMakeLists.txt
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(eventman SHARED
  event_man.cc
  trace_reader.cc
  event.cc
  easylogging++.cc
  config.cc
)
# set_target_properties(eventman PROPERTIES LINK_FLAGS "-nostartfiles")

target_link_libraries(eventman
  ${CMAKE_DL_LIBS}
  -lm
  dl
  stdc++
  json-c
  zstd
)

install(TARGETS ${PROJECT_NAME} DESTINATION "${LIBRARY_INSTALL_PREFIX}/tools")
install(TARGETS eventman DESTINATION "${LIBRARY_INSTALL_PREFIX}")


add_subdirectory(debugadapter)
add_subdirectory(languageadapter)
add_subdirectory(performancedapter)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686"
        OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64"
        OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "sw_64")
    message("eventrecorder disabled on ${CMAKE_SYSTEM_PROCESSOR}")
else()
    if (NOT DISABLE_MODULE)
        add_subdirectory(eventrecorder)
    endif()
endif()


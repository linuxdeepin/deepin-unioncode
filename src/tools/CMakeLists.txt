add_subdirectory(debugadapter)
add_subdirectory(languageadapter)
#add_subdirectory(performancedapter)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
        OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    if (NOT DISABLE_MODULE)
        add_subdirectory(eventrecorder)
    endif()
else()
    message("eventrecorder disabled on ${CMAKE_SYSTEM_PROCESSOR}")
endif()


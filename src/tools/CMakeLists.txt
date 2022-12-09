add_subdirectory(debugadapter)
add_subdirectory(languageadapter)
add_subdirectory(performancedapter)
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
    message("eventrecorder disabled on loongarch64")
else()
    add_subdirectory(eventrecorder)
endif()


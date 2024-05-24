cmake_minimum_required(VERSION 2.8)

project(%{ProjectName})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(${CMAKE_MAJOR_VERSION} LESS 3 OR (${CMAKE_MAJOR_VERSION} EQUAL 3 AND ${CMAKE_MINOR_VERSION} LESS 16))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    message(STATUS "Adding -g to CXX_FLAGS because CMake is less than 3.16")
endif()

add_executable(${PROJECT_NAME} "main.cpp")

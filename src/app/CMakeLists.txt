# Set the project name
project(app) 

# Specify C++ standard (optional, but recommended for modern C++ features)
set(CMAKE_CXX_STANDARD 17)  # Or 14, 20 depending on your project's requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# Find the required Qt5 modules
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Add the executable target
add_executable(${PROJECT_NAME} main.cpp)

# Add preprocessor definitions (if needed)
add_definitions(-DEXTEND_BACKTRACE)  

# Link libraries to the target
target_link_libraries(
    ${PROJECT_NAME}
    framework  
    common
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${DtkWidget_LIBRARIES}
)

# Install the executable (optional)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


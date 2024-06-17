# Set the project name
project(base)

# Define header and source files
set(CXX_H
    abstractmenu.h
    abstractaction.h
    abstractwidget.h
    abstractdebugger.h
    abstractlexerproxy.h
    abstractoutputparser.h
    abstractlocator.h
    basefilelocator.h
    baseitemdelegate.h
    abstracteditwidget.h
)

set(CXX_CPP
    abstractmenu.cpp
    abstractaction.cpp
    abstractwidget.cpp
    abstractdebugger.cpp
    abstractlexerproxy.cpp
    abstractoutputparser.cpp
    abstractlocator.cpp
    basefilelocator.cpp
    baseitemdelegate.cpp
    abstracteditwidget.cpp
)

# Add the shared library target
add_library(
    ${PROJECT_NAME}
    SHARED 
    ${CXX_H}
    ${CXX_CPP}
)

# Link with the DtkWidget library
target_link_libraries(
    ${PROJECT_NAME}
    ${DtkWidget_LIBRARIES}  # Assuming you have defined this variable elsewhere
)

# Set target properties (optional)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install the library (optional)
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# (Optional) Install header files
install(FILES ${CXX_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})


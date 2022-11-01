include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/qtermwidget-0.14.1/
    ${CMAKE_SOURCE_DIR}/3rdparty/qtermwidget-0.14.1/lib
    ${CMAKE_BINARY_DIR}/3rdparty/qtermwidget-0.14.1/lib
    )

set(CMAKE_CXX_STANDARD 17)

link_libraries(unioncode-qtermwidget)

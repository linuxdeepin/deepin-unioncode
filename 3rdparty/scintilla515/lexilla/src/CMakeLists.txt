project(lexilla)

set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(
    ../include
    ../lexlib
    ../../scintilla/include
    )

file(GLOB LEXLIB_SRC "../lexlib/*.cxx")
file(GLOB LEXERS_SRC "../lexers/*.cxx")
aux_source_directory(. SOURCES)

list(APPEND SOURCES ${LEXLIB_SRC} ${LEXERS_SRC})

set(HEADERS ../include/Lexilla.h)
file(GLOB LEXERS_INC "../lexers/*.h")
list(APPEND HEADERS ${LEXERS_INC})

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${HEADERS}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

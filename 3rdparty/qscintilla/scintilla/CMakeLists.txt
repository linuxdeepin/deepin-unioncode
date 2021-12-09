project(scintilla)

set(CXX_H

    )


set(CXX_CPP

    )

add_library(
    scintilla
    ${CXX_H}
    ${CXX_CPP})

message(${QtUseModules})
target_link_libraries(
    scintilla
    ${QtUseModules}
    )

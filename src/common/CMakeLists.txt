project(unioncode-common)

add_definitions(
    -DRUNTIME_INSTALL_RPEFIX="${RUNTIME_INSTALL_RPEFIX}"
    )

set(CXX_H
    inotify/inotify.h
    lsp/protocol.h
    util/processutil.h
    util/custompaths.h
    util/eventdefinitions.h
    util/singleton.h
    dialog/contextdialog.h
    widget/omittablelabel.h
    widget/collapsewidget.h
    widget/detailsbutton.h
    )

set(CXX_CPP
    inotify/inotify.cpp
    lsp/protocol.cpp
    util/processutil.cpp
    util/custompaths.cpp
    util/eventdefinitions.cpp
    dialog/contextdialog.cpp
    widget/omittablelabel.cpp
    widget/collapsewidget.cpp
    widget/detailsbutton.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})

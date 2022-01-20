project(unioncode-common)

configure_file(util/config.h.in config.h)

set(CXX_H
    inotify/inotify.h
    lsp/protocol.h
    util/processutil.h
    util/custompaths.h
    util/supportfile.h
    util/eventdefinitions.h
    util/singleton.h
    util/polkit.h
    dialog/contextdialog.h
    widget/collapsewidget.h
    widget/detailsbutton.h
    widget/statuswidget.h
    type/constants.h
    )

set(CXX_CPP
    inotify/inotify.cpp
    lsp/protocol.cpp
    util/processutil.cpp
    util/custompaths.cpp
    util/supportfile.cpp
    util/polkit.cpp
    util/eventdefinitions.cpp
    dialog/contextdialog.cpp
    widget/collapsewidget.cpp
    widget/detailsbutton.cpp
    widget/statuswidget.cpp
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

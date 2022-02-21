project(unioncode-common)

configure_file(util/config.h.in config.h)

set(CXX_H
    common.h
    inotify/inotify.h
    lsp/protocol.h
    lsp/client.h
    util/processutil.h
    util/custompaths.h
    util/eventdefinitions.h
    util/singleton.h
    util/polkit.h
    supportfile/editorstyle.h
    supportfile/language.h
    supportfile/windowstyle.h
    supportfile/builder.h
    util/markdownparser.cpp
    dialog/contextdialog.h
    widget/collapsewidget.h
    widget/detailsbutton.h
    widget/statuswidget.h
    widget/singlechoicebox.h
    widget/elidedlabel.h
    type/constants.h
    type/menuext.h
    )

set(CXX_CPP
    inotify/inotify.cpp
    lsp/protocol.cpp
    lsp/client.cpp
    util/processutil.cpp
    util/custompaths.cpp
    supportfile/language.cpp
    util/polkit.cpp
    util/eventdefinitions.cpp
    util/markdownparser.cpp
    supportfile/editorstyle.cpp
    supportfile/windowstyle.cpp
    supportfile/builder.cpp
    dialog/contextdialog.cpp
    widget/collapsewidget.cpp
    widget/detailsbutton.cpp
    widget/statuswidget.cpp
    widget/elidedlabel.cpp
    widget/singlechoicebox.cpp
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

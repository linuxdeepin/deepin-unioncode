project(unioncode-common)

add_definitions(
    -DLIBRARY_INSTALL_PREFIX="${LIBRARY_INSTALL_PREFIX}"
    -DSOURCES_INSTALL_RPEFIX="${SOURCES_INSTALL_RPEFIX}"
    -DRUNTIME_INSTALL_RPEFIX="${RUNTIME_INSTALL_RPEFIX}"
    )

set(CXX_H
    util/processutil.h
    util/custompaths.h
    util/eventdefinitions.h
    dialog/contextdialog.h
    widget/omittablelabel.h
    widget/collapsewidget.h
    widget/detailsbutton.h
    )

set(CXX_CPP
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

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})

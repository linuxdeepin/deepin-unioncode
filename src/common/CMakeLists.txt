project(unioncode-common)

set(CXX_H
    util/processutil.h
    dialog/contextdialog.h
    )

set(CXX_CPP
    util/processutil.cpp
    dialog/contextdialog.cpp
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${CXX_H}
    ${CXX_CPP}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_PREFIX})

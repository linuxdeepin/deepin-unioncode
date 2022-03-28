include($$PWD/aux_widgets/AuxiliarWidgets.pri)
include($$PWD/big_widgets/BigWidgets.pri)
include($$PWD/branches/Branches.pri)
include($$PWD/commits/Commits.pri)
include($$PWD/config/Config.pri)
include($$PWD/diff/Diff.pri)
include($$PWD/git/Git.pri)
include($$PWD/cache/Cache.pri)
include($$PWD/history/History.pri)
include($$PWD/git_server/GitServer.pri)
include($$PWD/git_server/GitServerWidgets.pri)
include($$PWD/QPinnableTabWidget/QPinnableTabWidget.pri)
include($$PWD/jenkins/Jenkins.pri)

RESOURCES += \
    $$PWD/resources.qrc

RC_FILE = $$PWD/resources.rc

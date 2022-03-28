cmake_minimum_required(VERSION 3.2.0)

project(GitQlient)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#private module AuxiliarWidgets
set(AuxiliarWidgets_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/aux_widgets)
include_directories(${AuxiliarWidgets_DIR})
set(AuxiliarWidgets_FORMS
    ${AuxiliarWidgets_DIR}/BranchDlg.ui
    ${AuxiliarWidgets_DIR}/CreateRepoDlg.ui
    ${AuxiliarWidgets_DIR}/CredentialsDlg.ui
    ${AuxiliarWidgets_DIR}/InitialRepoConfig.ui
    ${AuxiliarWidgets_DIR}/PomodoroConfigDlg.ui
    ${AuxiliarWidgets_DIR}/PullDlg.ui
    ${AuxiliarWidgets_DIR}/SquashDlg.ui
    )
set(AuxiliarWidgets_HEADERS
    ${AuxiliarWidgets_DIR}/BranchDlg.h
    ${AuxiliarWidgets_DIR}/ButtonLink.hpp
    ${AuxiliarWidgets_DIR}/CheckBox.h
    ${AuxiliarWidgets_DIR}/ClickableFrame.h
    ${AuxiliarWidgets_DIR}/Colors.h
    ${AuxiliarWidgets_DIR}/CommitInfoPanel.h
    ${AuxiliarWidgets_DIR}/ConflictButton.h
    ${AuxiliarWidgets_DIR}/CreateRepoDlg.h
    ${AuxiliarWidgets_DIR}/CredentialsDlg.h
    ${AuxiliarWidgets_DIR}/GitQlientUpdater.h
    ${AuxiliarWidgets_DIR}/Highlighter.h
    ${AuxiliarWidgets_DIR}/InitialRepoConfig.h
    ${AuxiliarWidgets_DIR}/PomodoroButton.h
    ${AuxiliarWidgets_DIR}/PomodoroConfigDlg.h
    ${AuxiliarWidgets_DIR}/ProgressDlg.h
    ${AuxiliarWidgets_DIR}/PullDlg.h
    ${AuxiliarWidgets_DIR}/SquashDlg.h
    ${AuxiliarWidgets_DIR}/WaitingDlg.h
    )
set(AuxiliarWidgets_SOURCES
    ${AuxiliarWidgets_DIR}/BranchDlg.cpp
    ${AuxiliarWidgets_DIR}/ButtonLink.cpp
    ${AuxiliarWidgets_DIR}/CheckBox.cpp
    ${AuxiliarWidgets_DIR}/ClickableFrame.cpp
    ${AuxiliarWidgets_DIR}/CommitInfoPanel.cpp
    ${AuxiliarWidgets_DIR}/ConflictButton.cpp
    ${AuxiliarWidgets_DIR}/CreateRepoDlg.cpp
    ${AuxiliarWidgets_DIR}/CredentialsDlg.cpp
    ${AuxiliarWidgets_DIR}/GitQlientUpdater.cpp
    ${AuxiliarWidgets_DIR}/Highlighter.cpp
    ${AuxiliarWidgets_DIR}/InitialRepoConfig.cpp
    ${AuxiliarWidgets_DIR}/PomodoroButton.cpp
    ${AuxiliarWidgets_DIR}/PomodoroConfigDlg.cpp
    ${AuxiliarWidgets_DIR}/ProgressDlg.cpp
    ${AuxiliarWidgets_DIR}/PullDlg.cpp
    ${AuxiliarWidgets_DIR}/SquashDlg.cpp
    ${AuxiliarWidgets_DIR}/WaitingDlg.cpp
    )

set(AuxiliarWidgets_ALL ${AuxiliarWidgets_FORMS} ${AuxiliarWidgets_HEADERS} ${AuxiliarWidgets_SOURCES})

#module bitWidgets
set(BigWidgets_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/big_widgets)
include_directories(${BigWidgets_DIR})
set(BigWidgets_FORMS
    ${BigWidgets_DIR}/ConfigWidget.ui
    )
set(BitWidgets_HEADERS
    ${BigWidgets_DIR}/BlameWidget.h
    ${BigWidgets_DIR}/ConfigWidget.h
    ${BigWidgets_DIR}/Controls.h
    ${BigWidgets_DIR}/DiffWidget.h
    ${BigWidgets_DIR}/GitQlient.h
    ${BigWidgets_DIR}/GitQlientRepo.h
    ${BigWidgets_DIR}/GitQlientSettings.h
    ${BigWidgets_DIR}/GitQlientStyles.h
    ${BigWidgets_DIR}/GitServerWidget.h
    ${BigWidgets_DIR}/HistoryWidget.h
    ${BigWidgets_DIR}/MergeWidget.h
    )
set(BitWidgets_SOURCES
    ${BigWidgets_DIR}/BlameWidget.cpp
    ${BigWidgets_DIR}/ConfigWidget.cpp
    ${BigWidgets_DIR}/Controls.cpp
    ${BigWidgets_DIR}/DiffWidget.cpp
    ${BigWidgets_DIR}/GitQlient.cpp
    ${BigWidgets_DIR}/GitQlientRepo.cpp
    ${BigWidgets_DIR}/GitQlientSettings.cpp
    ${BigWidgets_DIR}/GitQlientStyles.cpp
    ${BigWidgets_DIR}/GitServerWidget.cpp
    ${BigWidgets_DIR}/HistoryWidget.cpp
    ${BigWidgets_DIR}/MergeWidget.cpp
    )
set(BigWidgets_ALL ${BigWidgets_FORMS} ${BitWidgets_HEADERS} ${BitWidgets_SOURCES})

#module branches
set(Branches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/branches)
include_directories(${Branches_DIR})
set(Branches_FORMS
    ${Branches_DIR}/AddSubmoduleDlg.ui
    ${Branches_DIR}/AddSubtreeDlg.ui
    ${Branches_DIR}/TagDlg.ui
    )
set(Branches_HEADERS
    ${Branches_DIR}/AddRemoteDlg.h
    ${Branches_DIR}/AddSubmoduleDlg.h
    ${Branches_DIR}/AddSubtreeDlg.h
    ${Branches_DIR}/BranchContextMenu.h
    ${Branches_DIR}/BranchTreeWidget.h
    ${Branches_DIR}/BranchesViewDelegate.h
    ${Branches_DIR}/BranchesWidget.h
    ${Branches_DIR}/BranchesWidgetMinimal.h
    ${Branches_DIR}/GitQlientBranchItemRole.h
    ${Branches_DIR}/StashesContextMenu.h
    ${Branches_DIR}/SubmodulesContextMenu.h
    ${Branches_DIR}/TagDlg.h
    )
set(Branches_SOURCES
    ${Branches_DIR}/AddRemoteDlg.cpp
    ${Branches_DIR}/AddSubmoduleDlg.cpp
    ${Branches_DIR}/AddSubtreeDlg.cpp
    ${Branches_DIR}/BranchContextMenu.cpp
    ${Branches_DIR}/BranchTreeWidget.cpp
    ${Branches_DIR}/BranchesViewDelegate.cpp
    ${Branches_DIR}/BranchesWidget.cpp
    ${Branches_DIR}/BranchesWidgetMinimal.cpp
    ${Branches_DIR}/StashesContextMenu.cpp
    ${Branches_DIR}/SubmodulesContextMenu.cpp
    ${Branches_DIR}/TagDlg.cpp
    )
set(Branches_ALL ${Branches_FORMS} ${Branches_HEADERS} ${Branches_SOURCES})

#module commits
set(Commits_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/commits)
include_directories(${Commits_DIR})
set(Commits_FORMS
    ${Commits_DIR}/CommitChangesWidget.ui
    )
set(Commits_HEADERS
    ${Commits_DIR}/AmendWidget.h
    ${Commits_DIR}/CommitChangesWidget.h
    ${Commits_DIR}/CommitInfoWidget.h
    ${Commits_DIR}/FileContextMenu.h
    ${Commits_DIR}/FileListDelegate.h
    ${Commits_DIR}/FileListWidget.h
    ${Commits_DIR}/FileWidget.h
    ${Commits_DIR}/GitQlientRole.h
    ${Commits_DIR}/StagedFilesList.h
    ${Commits_DIR}/UnstagedMenu.h
    ${Commits_DIR}/WipWidget.h
    )
set(Commits_SOURCES
    ${Commits_DIR}/AmendWidget.cpp
    ${Commits_DIR}/CommitChangesWidget.cpp
    ${Commits_DIR}/CommitInfoWidget.cpp
    ${Commits_DIR}/FileContextMenu.cpp
    ${Commits_DIR}/FileListDelegate.cpp
    ${Commits_DIR}/FileListWidget.cpp
    ${Commits_DIR}/FileWidget.cpp
    ${Commits_DIR}/StagedFilesList.cpp
    ${Commits_DIR}/UnstagedMenu.cpp
    ${Commits_DIR}/WipWidget.cpp
    )
set(Commits_ALL ${Commits_FORMS} ${Commits_HEADERS} ${Commits_SOURCES})

#module Config
set(Config_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/config)
include_directories(${Config_DIR})
set(Config_FORMS
    ${Config_DIR}/GitConfigDlg.ui)
set(Config_HEADERS
    ${Config_DIR}/GeneralConfigDlg.h
    ${Config_DIR}/GitConfigDlg.h
    ${Config_DIR}/InitScreen.h
    )
set(Config_SOURCES
    ${Config_DIR}/GeneralConfigDlg.cpp
    ${Config_DIR}/GitConfigDlg.cpp
    ${Config_DIR}/InitScreen.cpp
    )
set(Config_ALL ${Config_FORMS} ${Config_HEADERS} ${Config_SOURCES})

#module Diff
set(Diff_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/diff)
include_directories(${Diff_DIR})
set(Diff_HEADERS
    ${Diff_DIR}/DiffHelper.h
    ${Diff_DIR}/DiffInfo.h
    ${Diff_DIR}/FileBlameWidget.h
    ${Diff_DIR}/FileDiffEditor.h
    ${Diff_DIR}/FileDiffHighlighter.h
    ${Diff_DIR}/FileDiffView.h
    ${Diff_DIR}/FileDiffWidget.h
    ${Diff_DIR}/FileEditor.h
    ${Diff_DIR}/FullDiffWidget.h
    ${Diff_DIR}/IDiffWidget.h
    ${Diff_DIR}/LineNumberArea.h
    )
set(Diff_SOURCES
    ${Diff_DIR}/FileBlameWidget.cpp
    ${Diff_DIR}/FileDiffEditor.cpp
    ${Diff_DIR}/FileDiffHighlighter.cpp
    ${Diff_DIR}/FileDiffView.cpp
    ${Diff_DIR}/FileDiffWidget.cpp
    ${Diff_DIR}/FileEditor.cpp
    ${Diff_DIR}/FullDiffWidget.cpp
    ${Diff_DIR}/IDiffWidget.cpp
    ${Diff_DIR}/LineNumberArea.cpp
    )
set(Diff_ALL ${Diff_HEADERS} ${Diff_SOURCES})

#module Git
set(Git_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/git)
include_directories(${Git_DIR})
set(Git_HEADERS
    ${Git_DIR}/AGitProcess.h
    ${Git_DIR}/GitAsyncProcess.h
    ${Git_DIR}/GitBase.h
    ${Git_DIR}/GitBranches.h
    ${Git_DIR}/GitCloneProcess.h
    ${Git_DIR}/GitConfig.h
    ${Git_DIR}/GitCredentials.h
    ${Git_DIR}/GitExecResult.h
    ${Git_DIR}/GitHistory.h
    ${Git_DIR}/GitLocal.h
    ${Git_DIR}/GitMerge.h
    ${Git_DIR}/GitPatches.h
    ${Git_DIR}/GitRemote.h
    ${Git_DIR}/GitRepoLoader.h
    ${Git_DIR}/GitRequestorProcess.h
    ${Git_DIR}/GitStashes.h
    ${Git_DIR}/GitSubmodules.h
    ${Git_DIR}/GitSubtree.h
    ${Git_DIR}/GitSyncProcess.h
    ${Git_DIR}/GitTags.h
    ${Git_DIR}/GitWip.h
    )
set(Git_SOURCES
    ${Git_DIR}/AGitProcess.cpp
    ${Git_DIR}/GitAsyncProcess.cpp
    ${Git_DIR}/GitBase.cpp
    ${Git_DIR}/GitBranches.cpp
    ${Git_DIR}/GitCloneProcess.cpp
    ${Git_DIR}/GitConfig.cpp
    ${Git_DIR}/GitCredentials.cpp
    ${Git_DIR}/GitExecResult.cpp
    ${Git_DIR}/GitHistory.cpp
    ${Git_DIR}/GitLocal.cpp
    ${Git_DIR}/GitMerge.cpp
    ${Git_DIR}/GitPatches.cpp
    ${Git_DIR}/GitRemote.cpp
    ${Git_DIR}/GitRepoLoader.cpp
    ${Git_DIR}/GitRequestorProcess.cpp
    ${Git_DIR}/GitStashes.cpp
    ${Git_DIR}/GitSubmodules.cpp
    ${Git_DIR}/GitSubtree.cpp
    ${Git_DIR}/GitSyncProcess.cpp
    ${Git_DIR}/GitTags.cpp
    ${Git_DIR}/GitWip.cpp
    )
set(Git_ALL ${Git_HEADERS} ${Git_SOURCES})

#module Cache
set(Cache_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/cache)
include_directories(${Cache_DIR})
set(Cache_HEADERS
    ${Cache_DIR}/CommitInfo.h
    ${Cache_DIR}/GitCache.h
    ${Cache_DIR}/GitServerCache.h
    ${Cache_DIR}/Lane.h
    ${Cache_DIR}/LaneType.h
    ${Cache_DIR}/References.h
    ${Cache_DIR}/RevisionFiles.h
    ${Cache_DIR}/WipRevisionInfo.h
    ${Cache_DIR}/lanes.h
    )
set(Cache_SOURCES
    ${Cache_DIR}/CommitInfo.cpp
    ${Cache_DIR}/GitCache.cpp
    ${Cache_DIR}/GitServerCache.cpp
    ${Cache_DIR}/Lane.cpp
    ${Cache_DIR}/References.cpp
    ${Cache_DIR}/RevisionFiles.cpp
    ${Cache_DIR}/lanes.cpp
    )
set(Cache_ALL ${Cache_HEADERS} ${Cache_SOURCES})

#module History
set(History_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/history)
include_directories(${History_DIR})
set(History_HEADERS
    ${History_DIR}/CommitHistoryColumns.h
    ${History_DIR}/CommitHistoryContextMenu.h
    ${History_DIR}/CommitHistoryModel.h
    ${History_DIR}/CommitHistoryView.h
    ${History_DIR}/RepositoryViewDelegate.h
    ${History_DIR}/ShaFilterProxyModel.h
    )
set(History_SOURCES
    ${History_DIR}/CommitHistoryContextMenu.cpp
    ${History_DIR}/CommitHistoryModel.cpp
    ${History_DIR}/CommitHistoryView.cpp
    ${History_DIR}/RepositoryViewDelegate.cpp
    ${History_DIR}/ShaFilterProxyModel.cpp
    )
set(History_ALL ${History_DIR} ${History_SOURCES})

#module gitServer
set(GitServer_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/git_server)
include_directories(${GitServer_DIR})
set(GitServer_HEADERS
    ${GitServer_DIR}/Comment.h
    ${GitServer_DIR}/Commit.h
    ${GitServer_DIR}/ConfigData.h
    ${GitServer_DIR}/GitHubRestApi.h
    ${GitServer_DIR}/GitLabRestApi.h
    ${GitServer_DIR}/IRestApi.h
    ${GitServer_DIR}/Issue.h
    ${GitServer_DIR}/Label.h
    ${GitServer_DIR}/Milestone.h
    ${GitServer_DIR}/Platform.h
    ${GitServer_DIR}/PullRequest.h
    ${GitServer_DIR}/User.h
    )
set(GitServer_SOURCES
    ${GitServer_DIR}/GitHubRestApi.cpp
    ${GitServer_DIR}/GitLabRestApi.cpp
    ${GitServer_DIR}/IRestApi.cpp
    )
set(GitServer_ALL ${GitServer_HEADERS} ${GitServer_SOURCES})

#module gitServerWidgets
#    ${GitServerWidgets_DIR}/CreatePullRequestDlg.ui
#    ${GitServerWidgets_DIR}/CreatePullRequestDlg.h
#    ${GitServerWidgets_DIR}/CreatePullRequestDlg.cpp
#    ${GitServerWidgets_DIR}/CreateIssueDlg.ui
#    ${GitServerWidgets_DIR}/CreateIssueDlg.h
#    ${GitServerWidgets_DIR}/CreateIssueDlg.cpp
set(GitServerWidgets_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/git_server)
include_directories(${GitServerWidgets_DIR})
set(GitServerWidgets_FORMS
    ${GitServerWidgets_DIR}/AddCodeReviewDialog.ui
    ${GitServerWidgets_DIR}/MergePullRequestDlg.ui
    ${GitServerWidgets_DIR}/ServerConfigDlg.ui
    )
set(GitServerWidgets_HEADERS
    ${GitServerWidgets_DIR}/AddCodeReviewDialog.h
    ${GitServerWidgets_DIR}/AGitServerItemList.h
    ${GitServerWidgets_DIR}/AvatarHelper.h
    ${GitServerWidgets_DIR}/CircularPixmap.h
    ${GitServerWidgets_DIR}/CodeReviewComment.h
    ${GitServerWidgets_DIR}/IssueDetailedView.h
    ${GitServerWidgets_DIR}/IssueItem.h
    ${GitServerWidgets_DIR}/IssuesList.h
    ${GitServerWidgets_DIR}/MergePullRequestDlg.h
    ${GitServerWidgets_DIR}/PrChangeListItem.h
    ${GitServerWidgets_DIR}/PrChangesList.h
    ${GitServerWidgets_DIR}/PrCommentsList.h
    ${GitServerWidgets_DIR}/PrCommitsList.h
    ${GitServerWidgets_DIR}/PrList.h
    ${GitServerWidgets_DIR}/ServerConfigDlg.h
    ${GitServerWidgets_DIR}/SourceCodeReview.h
    ${GitServerWidgets_DIR}/document.h
    )
#    ${GitServerWidgets_DIR}/previewpage.h
set(GitServerWidgets_SOURCES
    ${GitServerWidgets_DIR}/AddCodeReviewDialog.cpp
    ${GitServerWidgets_DIR}/AGitServerItemList.cpp
    ${GitServerWidgets_DIR}/CircularPixmap.cpp
    ${GitServerWidgets_DIR}/CodeReviewComment.cpp
    ${GitServerWidgets_DIR}/IssueDetailedView.cpp
    ${GitServerWidgets_DIR}/IssueItem.cpp
    ${GitServerWidgets_DIR}/IssuesList.cpp
    ${GitServerWidgets_DIR}/MergePullRequestDlg.cpp
    ${GitServerWidgets_DIR}/PrChangeListItem.cpp
    ${GitServerWidgets_DIR}/PrChangesList.cpp
    ${GitServerWidgets_DIR}/PrCommentsList.cpp
    ${GitServerWidgets_DIR}/PrCommitsList.cpp
    ${GitServerWidgets_DIR}/PrList.cpp
    ${GitServerWidgets_DIR}/ServerConfigDlg.cpp
    ${GitServerWidgets_DIR}/SourceCodeReview.cpp
    ${GitServerWidgets_DIR}/document.cpp
    )
#    ${GitServerWidgets_DIR}/previewpage.cpp
set(GitServerWidgets_ALL ${GitServerWidgets_FORMS} ${GitServerWidgets_HEADERS} ${GitServerWidgets_SOURCES})

#module QPinnableTabWidget
set(QPinnableTabWidget_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/QPinnableTabWidget)
include_directories(${QPinnableTabWidget_DIR})
set(QPinnableTabWidget_SOURCES
    ${QPinnableTabWidget_DIR}/FakeCloseButton.cpp
    ${QPinnableTabWidget_DIR}/RealCloseButton.cpp
    ${QPinnableTabWidget_DIR}/QPinnableTabWidget.cpp
    )
set(QPinnableTabWidget_HEADERS
    ${QPinnableTabWidget_DIR}/FakeCloseButton.h
    ${QPinnableTabWidget_DIR}/QPinnableTabWidget.h
    ${QPinnableTabWidget_DIR}/RealCloseButton.h
    ${QPinnableTabWidget_DIR}/qpinnabletabwidget_global.h
    )
set(QPinnableTabWidget_ALL ${QPinnableTabWidget_SOURCES} ${QPinnableTabWidget_HEADERS})

#module Jenkins
set(Jenkins_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/jenkins)
include_directories(${Jenkins_DIR})
set(Jenkins_SOURCES
    ${Jenkins_DIR}/BuildGeneralInfoFetcher.cpp
    ${Jenkins_DIR}/IFetcher.cpp
    ${Jenkins_DIR}/JenkinsJobPanel.cpp
    ${Jenkins_DIR}/JenkinsWidget.cpp
    ${Jenkins_DIR}/JobButton.cpp
    ${Jenkins_DIR}/JobContainer.cpp
    ${Jenkins_DIR}/JobDetailsFetcher.cpp
    ${Jenkins_DIR}/JobFetcher.cpp
    ${Jenkins_DIR}/RepoFetcher.cpp
    ${Jenkins_DIR}/StageFetcher.cpp
    )
set(Jenkins_HEADERS
    ${Jenkins_DIR}/BuildGeneralInfoFetcher.h
    ${Jenkins_DIR}/IFetcher.h
    ${Jenkins_DIR}/JenkinsJobInfo.h
    ${Jenkins_DIR}/JenkinsJobPanel.h
    ${Jenkins_DIR}/JenkinsViewInfo.h
    ${Jenkins_DIR}/JenkinsWidget.h
    ${Jenkins_DIR}/JobButton.h
    ${Jenkins_DIR}/JobContainer.h
    ${Jenkins_DIR}/JobDetailsFetcher.h
    ${Jenkins_DIR}/JobFetcher.h
    ${Jenkins_DIR}/RepoFetcher.h
    ${Jenkins_DIR}/StageFetcher.h
    )
set(Jenkins_ALL ${Jenkins_SOURCES} ${Jenkins_HEADERS})

#module log
set(Log_DIR ${CMAKE_CURRENT_SOURCE_DIR}/QLogger)
include_directories(${Log_DIR})
set(Log_SOURCES
    ${Log_DIR}/QLogger.cpp
    ${Log_DIR}/QLoggerWriter.cpp
    )
set(Log_HEADERS
    ${Log_DIR}/QLogger.h
    ${Log_DIR}/QLoggerLevel.h
    ${Log_DIR}/QLoggerWriter.h
    )
set(LOG_ALL ${Log_SOURCES} ${Log_HEADERS})

#execute_process(COMMAND git rev-parse --short HEAD
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    RESULT_VARIABLE resultVar
#    RESULTS_VARIABLE resultsVar
#    OUTPUT_VARIABLE outputVar
#    ERROR_VARIABLE errorVar
#    )
# from system execute end is "\n"
#string(REPLACE "\n" "" outputVar ${outputVar})

set(VERSION "c5c15f2") # from git HEAD
set(SHA_VER "c5c15f2") # from git HEAD

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_NO_JAVA_STYLE_ITERATORS
    -DQT_NO_CAST_TO_ASCII
    -DQT_RESTRICTED_CAST_FROM_ASCII
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_USE_QSTRINGBUILDER
    -DVER="${VERSION}"
    -DSHA_VER="${SHA_VER}"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${AuxiliarWidgets_ALL}
    ${BigWidgets_ALL}
    ${Branches_ALL}
    ${Commits_ALL}
    ${Config_ALL}
    ${Diff_ALL}
    ${Git_ALL}
    ${Cache_ALL}
    ${History_ALL}
    ${GitServer_ALL}
    ${GitServerWidgets_ALL}
    ${QPinnableTabWidget_ALL}
    ${Jenkins_ALL}
    ${LOG_ALL}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    )

target_link_libraries(${PROJECT_NAME} ${QtUseModules})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_PREFIX})

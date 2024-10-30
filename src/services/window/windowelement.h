// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWELEMENT_H
#define WINDOWELEMENT_H

#include <QAction>
#include <QMenu>
#include <QDialog>
#include <QTabWidget>

// window display elements
namespace dpfservice {

// MWNA = MainWindow Navigation
inline const QString MWNA_RECENT { QObject::tr("Recent") };
inline const QString MWNA_EDIT { QObject::tr("Edit") };
inline const QString MWNA_DEBUG { QObject::tr("Debug") };
inline const QString MWNA_GIT { QObject::tr("Git") };
inline const QString MWNA_SVN { QObject::tr("Svn") };
inline const QString MWNA_RUNTIME { QObject::tr("Runtime") };
inline const QString MWNA_CODEGEEX { QObject::tr("CodeGeeX") };
inline const QString MWNA_ADVANCEDSEARCH { QObject::tr("Advanced Search") };

// MWTG = MainWindow TopTool group
inline const QString MWTG_EDIT { "Edit" };
inline const QString MWTG_DEBUG { "Debug" };

// WN = window name
inline const QString WN_CONTEXTWIDGET = "contextWidget";
inline const QString WN_WORKSPACE = "workspaceWidget";
inline const QString WN_RIGHTSPACE = "rightspaceWidget";

// MWM = MW menu
// MWMFA = MWM File Action
inline const QString MWM_FILE { QMenu::tr("&File") };
inline const QString MWMFA_CXX { "C/C++" };
inline const QString MWMFA_JAVA { "Java" };
inline const QString MWMFA_PYTHON { "Python" };
inline const QString MWMFA_DOCUMENT_NEW { QAction::tr("New Document") };
inline const QString MWMFA_FOLDER_NEW { QAction::tr("New Folder") };
inline const QString MWMFA_OPEN_PROJECT { QAction::tr("Open Project") };
inline const QString MWMFA_OPEN_DOCUMENT { QAction::tr("Open Document") };
inline const QString MWMFA_OPEN_FILE { QAction::tr("Open File") };
inline const QString MWMFA_OPEN_RECENT_DOCUMENTS { QAction::tr("Open Recent Documents") };
inline const QString MWMFA_OPEN_RECENT_FOLDER { QAction::tr("Open Recent Folders") };
inline const QString MWMFA_NEW_FILE_OR_PROJECT { QAction::tr("New File or Project") };
inline const QString MWMFA_QUIT { QAction::tr("Quit") };

// MWMBA = MWM Build Action
inline const QString MWM_BUILD { QMenu::tr("&Build") };
inline const QString MWMBA_BUILD { QAction::tr("Build") };
inline const QString MWMBA_REBUILD { QAction::tr("Rebuild") };
inline const QString MWMBA_CLEAN { QAction::tr("Clean Cache") };
inline const QString MWMBA_CANCEL { QAction::tr("Cancel Build") };

// MWMDA = MWM Debug Action
inline const QString MWM_DEBUG { QMenu::tr("&Debug") };
inline const QString MWMDA_START_DEBUG { QAction::tr("Start Debugging") };
inline const QString MWMDA_ATTACH_DEBUG { QAction::tr("Attaching to Running Program") };
inline const QString MWMDA_RUNNING { QAction::tr("Running") };
inline const QString MWMDA_INTERRUPT { QAction::tr("Interrupt") };
inline const QString MWMDA_CONTINUE { QAction::tr("Continue") };
inline const QString MWMDA_REVERSE_CONTINUE { QAction::tr("Reverse Continue") };
inline const QString MWMDA_ABORT_DEBUGGING { QAction::tr("Abort Debugging") };
inline const QString MWMDA_RESTART_DEBUGGING { QAction::tr("Restart Debugging") };
inline const QString MWMDA_STEP_OVER { QAction::tr("Step Over") };
inline const QString MWMDA_STEP_IN { QAction::tr("Step In") };
inline const QString MWMDA_STEP_OUT { QAction::tr("Step Out") };
inline const QString MWMDA_STEP_BACK { QAction::tr("Step Back") };
inline const QString MWMDA_REMOTE_DEBUG { QAction::tr("Remote Debug") };

// MWMTA = MWM Tool Action
inline const QString MWM_TOOLS { QMenu::tr("&Tools") };
inline const QString MWMTA_SEARCH { QAction::tr("Search") };
inline const QString MWMTA_OPTIONS { QAction::tr("Options") };
inline const QString MWMTA_PLUGINS { QAction::tr("Plugins") };
inline const QString MWMTA_USR_ACTION_ANALYZE { QAction::tr("User Action Analyse")};
inline const QString MWMTA_BINARY_TOOLS { QAction::tr("Binary Tools")};
inline const QString MWMTA_VALGRIND_MEMCHECK { QAction::tr("Valgrind Memcheck")};
inline const QString MWMTA_VALGRIND_HELGRIND { QAction::tr("Valgrind Helgrind")};

inline const QString MWM_HELP { QMenu::tr("&Help") };
inline const QString MWM_REPORT_BUG { QAction::tr("Report Bug") };
inline const QString MWM_HELP_DOCUMENTS { QAction::tr("Help Documents") };
inline const QString MWM_ABOUT_PLUGINS { QAction::tr("About Plugins")};

// Others
inline const QString TERMINAL_TAB_TEXT { QTabWidget::tr("&Terminal") };
// MWCWT = MW CodeEditor Window Title
inline const QString MWCWT_SYMBOL {QTabWidget::tr("Symbol")};

} // namespace dpfservice

#endif // WINDOWELEMENT_H

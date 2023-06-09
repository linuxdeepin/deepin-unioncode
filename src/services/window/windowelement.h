#ifndef WINDOWELEMENT_H
#define WINDOWELEMENT_H

#include <QAction>
#include <QMenu>
#include <QDialog>
#include <QTabWidget>

// window display elements
// MW = MainWindow
namespace dpfservice {

inline const int MW_MIN_WIDTH { 1200 };
inline const int MW_MIN_HEIGHT { 800 };

inline const QString MWNA_RECENT { QAction::tr("Recent") };
inline const QString MWNA_EDIT { QAction::tr("Edit") };
inline const QString MWNA_DEBUG { QAction::tr("Debug") };
inline const QString MWNA_RUNTIME { QAction::tr("Runtime") };

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

inline const QString MWM_BUILD { QMenu::tr("&Build") };
inline const QString MWMBA_BUILD { QAction::tr("Build") };
inline const QString MWMBA_REBUILD { QAction::tr("Rebuild") };
inline const QString MWMBA_CLEAN { QAction::tr("Clean") };
inline const QString MWMBA_CANCEL { QAction::tr("Cancel") };

inline const QString MWM_DEBUG { QMenu::tr("&Debug") };
inline const QString MWMDA_START_DEBUG { QAction::tr("Start Debugging") };
inline const QString MWMDA_RUNNING { QAction::tr("Running") };
inline const QString MWMDA_INTERRUPT { QAction::tr("Interrupt") };
inline const QString MWMDA_CONTINUE { QAction::tr("Continue") };
inline const QString MWMDA_ABORT_DEBUGGING { QAction::tr("Abort Debugging") };
inline const QString MWMDA_RESTART_DEBUGGING { QAction::tr("Restart Debugging") };
inline const QString MWMDA_STEP_OVER { QAction::tr("Step Over") };
inline const QString MWMDA_STEP_IN { QAction::tr("Step In") };
inline const QString MWMDA_STEP_OUT { QAction::tr("Step Out") };

inline const QString MWM_TOOLS { QMenu::tr("&Tools") };
inline const QString MWMTA_SEARCH { QAction::tr("Search") };
inline const QString MWMTA_PACKAGE_TOOLS { QAction::tr("Package Tools") };
inline const QString MWMTA_VERSION_TOOLS { QAction::tr("Version Tools") };
inline const QString MWMTA_CODE_FORMATTING { QAction::tr("Code Formatting") };
inline const QString MWMTA_RUNTIME_ANALYSIS { QAction::tr("Runtime Analysis") };
inline const QString MWMTA_TEST { QAction::tr("Test") };
inline const QString MWMTA_PLUGINS { QAction::tr("Plugins") };
inline const QString MWMTA_OPTIONS { QAction::tr("Options") };
inline const QString MWMTM_SWITCH_LANGUAGE { QMenu::tr("Switch language") };
inline const QString MWMTA_ENGLISH { QAction::tr("English") };
inline const QString MWMTA_CHINESE { QAction::tr("Chinese") };
inline const QString MWMTA_USR_ACTION_ANALYZE { QAction::tr("User Action Analyse")};
inline const QString MWMTA_BINARY_TOOLS { QAction::tr("Binary Tools")};


inline const QString MWM_HELP { QMenu::tr("&Help") };
inline const QString MWM_REPORT_BUG { QAction::tr("Report Bug") };
inline const QString MWM_HELP_DOCUMENTS { QAction::tr("Help Documents") };
inline const QString MWM_ABOUT { QAction::tr("About") };
inline const QString MWM_ABOUT_PLUGINS { QAction::tr("About Plugins...")};

inline const QString DIALOG_OPEN_DOCUMENT_TITLE { QDialog::tr("Open Document") };
inline const QString CONSOLE_TAB_TEXT { QTabWidget::tr("&Console") };

inline const QString MWM_ANALYZE { QMenu::tr("&Analyze")};
inline const QString MWMAA_VALGRIND_MEMCHECK { QAction::tr("Valgrind Memcheck")};
inline const QString MWMAA_VALGRIND_HELGRIND { QAction::tr("Valgrind Helgrind")};
} // namespace dpfservice

#endif // WINDOWELEMENT_H

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
inline const QString MWMFA_OPEN_RECENT_DOCUMENTS { QAction::tr("Open Recent Documents") };
inline const QString MWMFA_OPEN_RECENT_FOLDER { QAction::tr("Open Recent Folders") };
inline const QString MWMFA_QUIT { QAction::tr("Quit") };

inline const QString MWM_BUILD { QMenu::tr("&Build") };
inline const QString MWM_DEBUG { QMenu::tr("&Debug") };

inline const QString MWM_TOOLS { QMenu::tr("&Tools") };
inline const QString MWMTA_SEARCH { QAction::tr("Search") };
inline const QString MWMTA_PACKAGE_TOOLS { QAction::tr("Package Tools") };
inline const QString MWMTA_VERSION_TOOLS { QAction::tr("Version Tools") };
inline const QString MWMTA_CODE_FORMATTING { QAction::tr("Code Formatting") };
inline const QString MWMTA_RUNTIME_ANALYSIS { QAction::tr("Runtime Analysis") };
inline const QString MWMTA_TEST { QAction::tr("Test") };
inline const QString MWMTA_PLUGINS { QAction::tr("Plugins") };
inline const QString MWMTA_OPTIONS { QAction::tr("Options") };

inline const QString MWM_HELP { QMenu::tr("&Help") };

inline const QString DIALOG_OPEN_DOCUMENT_TITLE { QDialog::tr("Open Document") };
inline const QString CONSOLE_TAB_TEXT { QTabWidget::tr("&Console") };

} // namespace dpfservice

#endif // WINDOWELEMENT_H

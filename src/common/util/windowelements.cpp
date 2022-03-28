#include "windowelements.h"
#include <QAction>
#include <QMenu>
#include <QDialog>

const int MW_MIN_WIDTH { 1200 };
const int MW_MIN_HEIGHT { 800 };

const QString MWNA_RECENT { QAction::tr("Recent") };
const QString MWNA_EDIT { QAction::tr("Edit") };
const QString MWNA_DEBUG { QAction::tr("Debug") };
const QString MWNA_RUNTIME { QAction::tr("Runtime") };

const QString MWM_FILE { QMenu::tr("File") };
const QString MWMFA_DOCUMENT_NEW { QAction::tr("New Document") };
const QString MWMFA_FOLDER_NEW { QAction::tr("New Folder") };
const QString MWMFA_OPEN_DOCUMENT { QAction::tr("Open Document") };
const QString MWMFA_OPEN_FOLDER { QAction::tr("Open Folder") };
const QString MWMFA_OPEN_RECENT_DOCUMENTS { QAction::tr("Open Recent Documents") };
const QString MWMFA_OPEN_RECENT_FOLDER { QAction::tr("Open Recent Folders") };
const QString MWMFA_QUIT { QAction::tr("Quit") };

const QString MWM_BUILD { QMenu::tr("Build") };

const QString MWM_DEBUG { QMenu::tr("Debug") };

const QString MWM_TOOLS { QMenu::tr("Tools") };
const QString MWMTA_SEARCH { QAction::tr("Search") };
const QString MWMTA_PACKAGE_TOOLS { QAction::tr("Package Tools") };
const QString MWMTA_VERSION_TOOLS { QAction::tr("Version Tools") };
const QString MWMTA_CODE_FORMATTING { QAction::tr("Code Formatting") };
const QString MWMTA_RUNTIME_ANALYSIS { QAction::tr("Runtime Analysis") };
const QString MWMTA_TEST { QAction::tr("Test") };
const QString MWMTA_PLUGINS { QAction::tr("Plugins") };
const QString MWMTA_OPTIONS { QAction::tr("Options") };

const QString MWM_HELP {QMenu::tr("Help")};

const QString DIALOG_OPEN_DOCUMENT_TITLE { QDialog::tr("Open Document") };
const QString DIALOG_OPEN_FOLDER_TITLE { QDialog::tr("Open Folder") };

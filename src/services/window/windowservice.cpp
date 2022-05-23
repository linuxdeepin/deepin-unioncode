/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "windowservice.h"

#include <QAction>
#include <QMenu>
#include <QDialog>
#include <QTabWidget>

namespace dpfservice {

const int MW_MIN_WIDTH { 1200 };
const int MW_MIN_HEIGHT { 800 };

const QString MWNA_RECENT { QAction::tr("Recent") };
const QString MWNA_EDIT { QAction::tr("Edit") };
const QString MWNA_DEBUG { QAction::tr("Debug") };
const QString MWNA_RUNTIME { QAction::tr("Runtime") };

const QString MWM_FILE { QMenu::tr("File") };
const QString MWMFA_CXX { "C/C++" };
const QString MWMFA_JAVA { "Java" };
const QString MWMFA_PYTHON { "Python" };
const QString MWMFA_DOCUMENT_NEW { QAction::tr("New Document") };
const QString MWMFA_FOLDER_NEW { QAction::tr("New Folder") };
const QString MWMFA_OPEN_PROJECT { QAction::tr("Open Project") };
const QString MWMFA_OPEN_DOCUMENT { QAction::tr("Open Document") };
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

const QString MWM_HELP { QMenu::tr("Help") };

const QString DIALOG_OPEN_DOCUMENT_TITLE { QDialog::tr("Open Document") };
const QString CONSOLE_TAB_TEXT { QTabWidget::tr("Console") };

WindowService::WindowService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString WindowService::name()
{
    return "org.deepin.service.WindowService";
}

} // namespace dpfservice

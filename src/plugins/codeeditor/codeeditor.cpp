/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "codeeditor.h"
#include "mainframe/naveditmainwindow.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"

#include "refactorwidget/refactorwidget.h"
#include "textedittabwidget/texteditkeeper.h"
#include "textedittabwidget/textedittabwidget.h"
#include "textedittabwidget/language/texteditcpp.h"
#include "textedittabwidget/language/texteditcmake.h"

#include "services/window/windowservice.h"

#include <QAction>

using namespace dpfservice;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

void CodeEditor::initialize()
{
    qInfo() << __FUNCTION__;
    TextEditKeeper::impl<TextEdit>("");
    TextEditKeeper::impl<TextEditCpp>(TextEditCpp::implLanguage());
    TextEditKeeper::impl<TextEditCmake>(TextEditCmake::implLanguage());
}

bool CodeEditor::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        NavEditMainWindow *navEditWindow = NavEditMainWindow::instance();
        TextEditTabWidget *editTabWidget = TextEditTabWidget::instance();
        navEditWindow->setWidgetEdit(new AbstractCentral(editTabWidget));
        windowService->addCentralNavigation(MWNA_EDIT, new AbstractCentral(navEditWindow));

        using namespace std::placeholders;
        if (!windowService->addWidgetWorkspace) {
            windowService->addWidgetWorkspace = std::bind(&NavEditMainWindow::addWidgetWorkspace, navEditWindow, _1, _2);
        }

        if (!windowService->setWidgetConsole) {
            windowService->setWidgetConsole = std::bind(&NavEditMainWindow::setConsole, navEditWindow, _1);
        }

        if (!windowService->addContextWidget) {
            windowService->addContextWidget = std::bind(&NavEditMainWindow::addWidgetContext, navEditWindow, _1, _2);
        }

        navEditWindow->addWidgetContext(QTabWidget::tr("Code Lens"), new AbstractWidget(RefactorWidget::instance()));

        if (!windowService->setWidgetWatch) {
            windowService->setWidgetWatch = std::bind(&NavEditMainWindow::setWidgetWatch, navEditWindow, _1);
        }

        if (!windowService->switchWidgetContext) {
            windowService->switchWidgetContext = std::bind(&NavEditMainWindow::switchWidgetContext, navEditWindow, _1);
        }

        if (!windowService->switchWidgetWorkspace) {
            windowService->switchWidgetWorkspace = std::bind(&NavEditMainWindow::switchWidgetWorkspace, navEditWindow, _1);
        }

        auto saveAllDocuments = new QAction(SAVE_ALL_DOCUMENTS);
        ActionManager::getInstance()->registerAction(saveAllDocuments, "File.Save.All.Documents",
                                                     SAVE_ALL_DOCUMENTS, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_S));
        windowService->addAction(MWM_FILE, new AbstractAction(saveAllDocuments));

        auto closeAllDocuments = new QAction(CLOSE_ALL_DOCUMENTS);
        ActionManager::getInstance()->registerAction(closeAllDocuments, "File.Close.All.Documents",
                                                     CLOSE_ALL_DOCUMENTS, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_W));
        windowService->addAction(MWM_FILE, new AbstractAction(closeAllDocuments));

        auto sep = new QAction();
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        auto print = new QAction(PRINT);
        ActionManager::getInstance()->registerAction(print, "File.Print",
                                                     PRINT, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_P));
        windowService->addAction(MWM_FILE, new AbstractAction(print));

        saveAllDocuments->setEnabled(false);
        closeAllDocuments->setEnabled(false);
        print->setEnabled(false);
    }

    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


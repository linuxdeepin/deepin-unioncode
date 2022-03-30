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
#include "textedittabwidget/textedittabwidget.h"
#include "textedittabwidget/style/stylekeeper.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "services/window/windowservice.h"

#include "textedittabwidget/style/language/stylescicpp.h"
#include "textedittabwidget/style/language/stylescicmake.h"
#include "textedittabwidget/style/language/stylelspcpp.h"
#include "textedittabwidget/style/language/stylelspcmake.h"

#include <QAction>

using namespace dpfservice;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

void CodeEditor::initialize()
{
    qInfo() << __FUNCTION__;
    StyleKeeper::regClass<StyleSciCpp, StyleLspCpp>("cpp");
    StyleKeeper::regClass<StyleSciCmake, StyleLspCmake>("cmake");
    StyleKeeper::regClass<StyleSci, StyleLsp>("json");
}

bool CodeEditor::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        NavEditMainWindow *navEditWindow = NavEditMainWindow::instance();
        TextEditTabWidget *editTabWidget = TextEditTabWidget::instance();
        navEditWindow->setEditWidget(new AbstractCentral(editTabWidget));
        windowService->addCentral(MWNA_EDIT, new AbstractCentral(navEditWindow));

        using namespace std::placeholders;
        if (!windowService->setEditorTree) {
            windowService->setEditorTree = std::bind(&NavEditMainWindow::setTreeWidget, navEditWindow, _1);
        };

        if (!windowService->setEditorConsole) {
            windowService->setEditorConsole = std::bind(&NavEditMainWindow::setConsole, navEditWindow, _1);
        }

        if (!windowService->addContextWidget) {
            windowService->addContextWidget = std::bind(&NavEditMainWindow::addContextWidget, navEditWindow, _1, _2);
        }

        if (!windowService->setWatchWidget) {
            windowService->setWatchWidget = std::bind(&NavEditMainWindow::setWatchWidget, navEditWindow, _1);
        }

        if (!windowService->switchContextWidget) {
            windowService->switchContextWidget = std::bind(&NavEditMainWindow::switchContextWidget, navEditWindow, _1);
        }

        auto saveAllDocuments = new QAction(SAVE_ALL_DOCUMENTS);
        windowService->addAction(MWM_FILE, new AbstractAction(saveAllDocuments));

        auto closeAllDocuments = new QAction(CLOSE_ALL_DOCUMENTS);
        windowService->addAction(MWM_FILE, new AbstractAction(closeAllDocuments));

        auto sep = new QAction();
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        auto print = new QAction(PRINT);
        windowService->addAction(MWM_FILE, new AbstractAction(print));
    }
    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


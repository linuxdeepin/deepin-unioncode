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
#include "naveditwidget.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "editfilestatusbar.h"

#include <QAction>

using namespace dpfservice;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

void CodeEditor::initialize()
{
    qInfo() << __FUNCTION__;
}

bool CodeEditor::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        NavEditWidget *navEditWidget = new NavEditWidget;

        windowService->addCentral(QString::fromStdString(NAVACTION_EDIT),
                                  new AbstractCentral(navEditWidget));

        using namespace std::placeholders;
        if (!windowService->setEditorTree) {
            windowService->setEditorTree = std::bind(&NavEditWidget::setTreeWidget, navEditWidget, _1);
        };

        if (!windowService->setEditorConsole) {
            windowService->setEditorConsole = std::bind(&NavEditWidget::setConsole, navEditWidget, _1);
        }

        if (!windowService->addContextWidget) {
            windowService->addContextWidget = std::bind(&NavEditWidget::addContextWidget, navEditWidget, _1, _2);
        }

        auto saveAllDocuments = new QAction(SAVE_ALL_DOCUMENTS);
        windowService->addAction(QString::fromStdString(MENU_FILE),
                                 new AbstractAction(saveAllDocuments));

        auto closeAllDocuments = new QAction(CLOSE_ALL_DOCUMENTS);
        windowService->addAction(QString::fromStdString(MENU_FILE),
                                 new AbstractAction(closeAllDocuments));

        auto sep = new QAction();
        sep->setSeparator(true);
        windowService->addAction(QString::fromStdString(MENU_FILE),
                                 new AbstractAction(sep));

        auto print = new QAction(PRINT);
        windowService->addAction(QString::fromStdString(MENU_FILE),
                                 new AbstractAction(print));
    }
    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


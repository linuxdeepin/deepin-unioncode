/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "codelens/codelens.h"
#include "textedittabwidget/textedittabwidget.h"
#include "textedittabwidget/language/cpp/texteditcpp.h"
#include "textedittabwidget/language/cmake/texteditcmake.h"
#include "textedittabwidget/language/java/texteditjava.h"
#include "textedittabwidget/language/python/texteditpython.h"
#include "textedittabwidget/language/js/texteditjs.h"
#include "textedittabwidget/texteditsplitter.h"
#include "mainframe/naveditmainwindow.h"
#include "mainframe/texteditkeeper.h"
#include "transceiver/codeeditorreceiver.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"

#include "services/window/windowservice.h"
#include "services/language/languageservice.h"
#include "common/widget/outputpane.h"

#include <QAction>
#include <QSplitter>

using namespace dpfservice;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

void CodeEditor::initialize()
{
    qInfo() << __FUNCTION__;
    TextEditKeeper::impl<TextEdit>("");
    TextEditKeeper::impl<TextEditPython>();
    TextEditKeeper::impl<TextEditCpp>();
    TextEditKeeper::impl<TextEditCmake>();
    TextEditKeeper::impl<TextEditJava>();
    TextEditKeeper::impl<TextEditJS>();

    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(dpfservice::LanguageService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool CodeEditor::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        NavEditMainWindow *navEditWindow = NavEditMainWindow::instance();
        TextEditSplitter *editManager = TextEditSplitter::instance();
        navEditWindow->setWidgetEdit(new AbstractCentral(editManager));
        windowService->addCentralNavigation(MWNA_EDIT, new AbstractCentral(navEditWindow));

        using namespace std::placeholders;
        if (!windowService->addWidgetWorkspace) {
            windowService->addWidgetWorkspace = std::bind(&NavEditMainWindow::addWidgetWorkspace, navEditWindow, _1, _2);
        }

        if (!windowService->addContextWidget) {
            windowService->addContextWidget = std::bind(&NavEditMainWindow::addContextWidget, navEditWindow, _1, _2, _3);
        }

        if (!windowService->removeContextWidget) {
            windowService->removeContextWidget = std::bind(&NavEditMainWindow::removeContextWidget, navEditWindow, _1);
        }

        if (!windowService->setWidgetEdit) {
            windowService->setWidgetEdit = std::bind(&NavEditMainWindow::setWidgetEdit, navEditWindow, _1);
        }

        windowService->addContextWidget(QTabWidget::tr("Code &Lens"), new AbstractWidget(CodeLens::instance()), "Lens");

        if (!windowService->setWidgetWatch) {
            windowService->setWidgetWatch = std::bind(&NavEditMainWindow::setWidgetWatch, navEditWindow, _1);
        }

        if (!windowService->addFindToolBar) {
            windowService->addFindToolBar = std::bind(&NavEditMainWindow::addFindToolBar, navEditWindow, _1);
        }

        if (!windowService->showFindToolBar) {
            windowService->showFindToolBar = std::bind(&NavEditMainWindow::showFindToolBar, navEditWindow);
        }

        auto sep = new QAction();
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        windowService->addContextWidget(tr("&Application Output"), new AbstractWidget(OutputPane::instance()), "Application");
    }

    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


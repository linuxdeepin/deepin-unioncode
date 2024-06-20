// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbol.h"
#include "mainframe/symbolkeeper.h"
#include "mainframe/symboltreeview.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include <DTreeView>

#include <QProcess>
#include <QAction>
#include <QLabel>

using namespace dpfservice;
void Symbol::initialize()
{
    QProcess process;
    process.start("pip3 show esprima");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    if(output.isEmpty()) {
        process.start("pip3 install esprima");
        process.waitForFinished();
    }
}

bool Symbol::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addWidgetWorkspace) {
            auto view = new AbstractWidget(SymbolKeeper::instance()->treeView());
            windowService->addWidgetWorkspace(MWCWT_SYMBOL, view, "edit-tree");
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag Symbol::stop()
{
    return Sync;
}

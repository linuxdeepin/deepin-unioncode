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
#include "symbol.h"
#include "mainframe/symbolkeeper.h"
#include "mainframe/symboltreeview.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/symbol/symbolservice.h"

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>

using namespace dpfservice;
void Symbol::initialize()
{

}

bool Symbol::start()
{
    qInfo() << __FUNCTION__;
    QString unionparser = "unionparser";
    if (!ProcessUtil::exists(unionparser)) {
        if (env::pkg::native::installed()) {
            QString parserNativePkgPath = env::pkg::native::path(env::pkg::Category::get()->unionparser);
            if (QFileInfo(parserNativePkgPath).isFile()) {
                ProcessUtil::execute("pip3", {"install", parserNativePkgPath}, [=](const QByteArray &data){
                    qInfo() << qPrintable(data);
                });
            } else {
                qCritical() << "Failed, Not found unionparser env package to install!!!";
            }
        }
    }

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addWidgetWorkspace) {
            auto view = new AbstractWidget(SymbolKeeper::instance()->treeView());
            windowService->addWidgetWorkspace(MWCWT_SYMBOL, view);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag Symbol::stop()
{
    return Sync;
}

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
#include "symbolcpp.h"
#include "mainframe/symbolcppgenerator.h"
#include "transceiver/sendevents.h"

#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>

using namespace dpfservice;
void SymbolCpp::initialize()
{
    qInfo() << __FUNCTION__;
}

bool SymbolCpp::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    using namespace std::placeholders;
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        projectService->implGenerator<SymbolCppGenerator>(MWMFA_CXX);
    }
    return true;
}

dpf::Plugin::ShutdownFlag SymbolCpp::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

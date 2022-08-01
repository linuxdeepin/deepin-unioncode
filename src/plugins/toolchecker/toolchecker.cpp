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
#include "toolchecker.h"
#include "mainframe/backendchecker.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

#include <QAction>
#include <QLabel>
#include <QDialog>
#include <QProgressBar>

using namespace dpfservice;
namespace {
    BackendChecker *check;
}
void ToolChecker::initialize()
{

}

bool ToolChecker::start()
{
    qInfo() << __FUNCTION__;
    // TODO(huangyu):Open it when refactor done.
//    check = new BackendChecker;
    return true;
}

dpf::Plugin::ShutdownFlag ToolChecker::stop()
{
    delete check;
    return Sync;
}

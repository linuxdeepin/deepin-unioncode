/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "javascriptplugin.h"
#include "jsgenerator.h"
#include "project/jsprojectgenerator.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"
#include "services/debugger/debuggerservice.h"
#include "base/abstractwidget.h"
#include "base/abstractcentral.h"
#include "debugger/jsdebugger.h"

#include <QtWidgets/QAction>
#include <QtScript>
#ifndef QT_NO_SCRIPTTOOLS
#include <QtScriptTools>
#endif
#include <QMainWindow>
#include <QApplication>

using namespace dpfservice;

void JavascriptPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool JavascriptPlugin::start()
{
    qInfo() << __FUNCTION__;
    // language register.
    LanguageService *languageService = dpfGetService(LanguageService);
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<JSGenerator>(JSGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<JSGenerator>(JSGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }

    // project register.
    ProjectService *projectService = dpfGetService(ProjectService);
    if (projectService) {
        QString errorString;
        projectService->implGenerator<JSProjectGenerator>(JSProjectGenerator::toolKitName(), &errorString);
    }

    // debugger register.
    auto jsDebugger = new JSDebugger();
    auto debuggerService = dpfGetService(DebuggerService);
    if (debuggerService && debuggerService->registerDebugger) {
        debuggerService->registerDebugger("local", jsDebugger);
    }

    return true;
}

dpf::Plugin::ShutdownFlag JavascriptPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}



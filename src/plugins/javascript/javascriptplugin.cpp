// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "javascriptplugin.h"
#include "jsgenerator.h"
#include "project/jsprojectgenerator.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"
#include "services/debugger/debuggerservice.h"
#include "base/abstractwidget.h"
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



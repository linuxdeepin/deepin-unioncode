// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "linglongplugin.h"
#include "llglobal.h"
#include "llgenerator.h"
#include "project/llprojectgenerator.h"
#include "builder/mainframe/llbuildergenerator.h"

#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"

#include "gui/mainframe.h"

using namespace dpfservice;

void LinglongPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool LinglongPlugin::start()
{
    qInfo() << __FUNCTION__;
    // language register.
    LanguageService *languageService = dpfGetService(LanguageService);
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<LLGenerator>(LLGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<LLGenerator>(LLGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }

    // project register.
    ProjectService *projectService = dpfGetService(ProjectService);
    if (projectService) {
        QString errorString;
        projectService->implGenerator<LLProjectGenerator>(LLProjectGenerator::toolKitName(), &errorString);
    }

    //builder register
    BuilderService *builderService = dpfGetService(BuilderService);
    if (builderService) {
        QString errorString;
        bool ret = builderService->regClass<LLBuilderGenerator>(LLBuilderGenerator::toolKitName(), &errorString);
        if (ret) {
            builderService->create<LLBuilderGenerator>(LLBuilderGenerator::toolKitName(), &errorString);
        }
    }

    QAction *action = new QAction(QIcon::fromTheme("linglong"), LL_NAME, this);
    windowService = dpfGetService(WindowService);
    if (!windowService)
        return false;

    windowService->addNavigationItem(new AbstractAction(action), Priority::low);
    auto mainFrame = new AbstractWidget(new MainFrame());
    // mainFrame will setParent when register to windowService
    windowService->registerWidget(LL_NAME, mainFrame);

    connect(action, &QAction::triggered, this, [=]() {
        windowService->showWidgetAtPosition(LL_NAME, Position::FullWindow, true);
        windowService->showContextWidget();
    },
            Qt::DirectConnection);

    return true;
}

dpf::Plugin::ShutdownFlag LinglongPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

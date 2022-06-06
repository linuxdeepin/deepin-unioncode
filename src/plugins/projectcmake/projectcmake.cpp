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
#include "projectcmake.h"
#include "mainframe/cmakeopenhandler.h"
#include "mainframe/cmakegenerator.h"
#include "transceiver/sendevents.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QAction>
#include <QLabel>

using namespace dpfservice;

void ProjectCMake::initialize()
{
}

bool ProjectCMake::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    // 注册生成器
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
    }

    // 注册工程打开后续逻辑
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        CMakeOpenHandler *openHandler = CMakeOpenHandler::instance();
        QObject::connect(openHandler, &CMakeOpenHandler::projectOpened,
                         [=](const QString &name, const QString &language, const QString &filePath) {
            // 打开工程后续流程
            if (projectService) {
                ProjectGenerator *generator = projectService->createGenerator<ProjectGenerator>(name);
                QString projectFilePath = filePath;
                // show build type config pane.
                projectService->showConfigureProjDlg(projectFilePath);
                // get config result.
                QString outputPath = projectService->getDefaultOutputPath();

                QObject::connect(generator, &ProjectGenerator::targetExecute,
                                 [=](const QString &cmd, const QStringList &args) {
                    // Execute project tree command.
                    emit projectService->targetCommand(cmd, args);
                });

                ProjectInfo info;
                QString sourceFolder = QFileInfo(filePath).path();
                info.setLanguage(language);
                info.setSourceFolder(sourceFolder);
                info.setKitName(CMakeGenerator::toolKitName());
                info.setBuildFolder(outputPath);
                info.setWorkspaceFolder(sourceFolder);
                info.setProjectFilePath(filePath);
                info.setBuildType("Debug");
                info.setBuildCustomArgs({"-DCMAKE_EXPORT_COMPILE_COMMANDS=1"});

                auto rootItem = generator->createRootItem(info);
                if (rootItem) {
                    SendEvents::menuOpenProject(filePath); // 发送打开事件
                    if (projectService->projectView.addRootItem)
                        projectService->projectView.addRootItem(rootItem);   // 设置项目根节点
                    if (projectService->projectView.expandedDepth)
                        projectService->projectView.expandedDepth(rootItem, 2);   // 初始化展开两级
                    if (windowService->switchWidgetNavigation)
                        windowService->switchWidgetNavigation(MWNA_EDIT);   // 切换编辑器导航栏
                    if (windowService->switchWidgetWorkspace)
                        windowService->switchWidgetWorkspace(MWCWT_PROJECTS);

                    emit projectService->projectConfigureDone();
                } else {
                    ContextDialog::ok(QDialog::tr("Open Project Error: %0").arg(filePath));
                }
            }
        });

        if (windowService->addOpenProjectAction) {
            windowService->addOpenProjectAction(MWMFA_CXX, new AbstractAction(openHandler->openAction()));
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag ProjectCMake::stop()
{
    return Sync;
}

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
                         [=](const QString &name, const QString &filePath) {

            dpf::Event menuOpenFile;
            menuOpenFile.setTopic(T_MENU);
            menuOpenFile.setData(D_FILE_OPENFOLDER);
            QFileInfo fileInfo(filePath);
            menuOpenFile.setProperty(P_FILEPATH, fileInfo.dir().path());
            dpf::EventCallProxy::instance().pubEvent(menuOpenFile);

            if (projectService) {
                ProjectGenerator *generator = projectService->createGenerator(name);
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
                auto rootItem = generator->createRootItem(filePath, outputPath);
                if (projectService->addProjectRootItem)
                    projectService->addProjectRootItem(rootItem);   // 设置项目根节点
                if (projectService->expandedProjectDepth)
                    projectService->expandedProjectDepth(rootItem, 2);   // 初始化展开两级
                if (windowService->switchWidgetNavigation)
                    windowService->switchWidgetNavigation(MWNA_EDIT);   // 切换编辑器导航栏
                if (windowService->switchWidgetWorkspace)
                    windowService->switchWidgetWorkspace(MWCWT_PROJECTS);

                emit projectService->projectConfigureDone();
            }
        });

        if (windowService->addOpenProjectAction) {
            windowService->addOpenProjectAction(new AbstractAction(openHandler->openAction()));
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag ProjectCMake::stop()
{
    return Sync;
}

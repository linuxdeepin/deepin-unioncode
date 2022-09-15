/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "cmakeopenhandler.h"
#include "cmakegenerator.h"
#include "transceiver/sendevents.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/toolchecker/toolcheckerservice.h"

#include "base/abstractaction.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

namespace {
CMakeOpenHandler *ins {nullptr};
}

CMakeOpenHandler::CMakeOpenHandler(QObject *parent) : QObject(parent)
{

}

CMakeOpenHandler *CMakeOpenHandler::instance()
{
    if (!ins)
        ins = new CMakeOpenHandler;
    return ins;
}

QAction *CMakeOpenHandler::openAction()
{
    auto result = new QAction(CmakeGenerator::toolKitName());

    ActionManager::getInstance()->registerAction(result,
                                                 "File.Open.Project",
                                                 result->text(),
                                                 QKeySequence(Qt::Modifier::CTRL |
                                                              Qt::Modifier::SHIFT |
                                                              Qt::Key::Key_O));

    QObject::connect(result, &QAction::triggered, [=](){

        QString iniPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("setting.ini");
        QSettings setting(iniPath, QSettings::IniFormat);
        QString lastPath = setting.value("cmakelastpath").toString();

        QFileDialog fileDialog;
        QString filePath = fileDialog.getOpenFileName(nullptr, "Open CMake Project", lastPath, "CMakeLists.txt");
        if(!filePath.isEmpty()) {

            // save open history
            QString lastPath = filePath.left(filePath.lastIndexOf('/'));
            QVariant varPath(lastPath);
            setting.setValue("cmakelastpath", varPath);

            QString kitName = result->text();
            QString language = dpfservice::MWMFA_CXX;
            doProjectOpen(kitName, language, filePath);
        }
    });
    return result;
}

void CMakeOpenHandler::doProjectOpen(const QString &name, const QString &language, const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    ToolCheckerSevice *toolCheckerService = ctx.service<ToolCheckerSevice>(ToolCheckerSevice::name());
    if (!projectService || !windowService || !toolCheckerService)
        return;

    if (toolCheckerService->checkLanguageBackend)
        toolCheckerService->checkLanguageBackend(language);

    auto generator = projectService->createGenerator<CmakeGenerator>(name);
    if (!generator)
        return;

    auto configWidget = generator->configureWidget(language, filePath);
    if (configWidget) {
        configWidget->setWindowFlags(configWidget->windowFlags() | Qt::Tool | Qt::Dialog);
        configWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        configWidget->setWindowModality(Qt::ApplicationModal);
        configWidget->setAttribute(Qt::WA_ShowModal, true);
        QDesktopWidget *desktop = QApplication::desktop();
        configWidget->move((desktop->width() - configWidget->width()) / 2,
                           (desktop->height() - configWidget->height()) / 2);
        configWidget->show();
    }
}

void CMakeOpenHandler::doActiveProject(const QString &buildDirectory)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    emit projectService->projectConfigureDone(buildDirectory);
}

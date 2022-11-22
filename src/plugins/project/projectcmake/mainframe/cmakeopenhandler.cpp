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
#include "properties/configutil.h"
#include "properties/targetsmanager.h"

#include "services/window/windowservice.h"
#include "services/toolchecker/toolcheckerservice.h"

#include "base/abstractaction.h"
#include "base/abstractwidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QComboBox>

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

    config::ConfigureParam *param = config::ConfigUtil::instance()->getConfigureParamPointer();
    if (config::ConfigUtil::instance()->isNeedConfig(QFileInfo(filePath).path(), *param)) {
        auto configWidget = generator->configureWidget(language, filePath);
        if (configWidget) {
            configWidget->exec();
        }
    } else {
        ProjectInfo info;
        if (config::ConfigUtil::instance()->getProjectInfo(param, info)) {
            generator->configure(info);
            TargetsManager::instance()->initialize(info.buildFolder());
        }
    }
}

void CMakeOpenHandler::doActiveProject(const dpfservice::ProjectInfo &projectInfo)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    emit projectService->projectConfigureDone(projectInfo.buildFolder());
    if (projectInfo.kitName() == CmakeGenerator::toolKitName()) {
        adjustToolBar(true);
    }
}

void CMakeOpenHandler::doDeleteProject(const dpfservice::ProjectInfo &projectInfo)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    if (projectInfo.kitName() == CmakeGenerator::toolKitName()) {
        adjustToolBar(false);
    }
}

void CMakeOpenHandler::adjustToolBar(bool bAdd)
{
    const QString buildTypeKey = "CMake.BuildType";
    const QString runKey = "CMake.Run";
    auto &ctx = dpfInstance.serviceContext();
    using namespace dpfservice;
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    if (bAdd) {
        QComboBox *comboBox = new QComboBox();
        comboBox->setEditable(false);
        comboBox->insertItem(0, "Debug");
        comboBox->insertItem(1, "Release");

        config::ConfigureParam *param = config::ConfigUtil::instance()->getConfigureParamPointer();
        if (config::ConfigType::Debug == param->defaultType) {
            comboBox->setCurrentIndex(0);
        } else if (config::ConfigType::Release == param->defaultType) {
            comboBox->setCurrentIndex(1);
        } else {
            comboBox->setCurrentIndex(-1);
        }

        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
            qInfo() << comboBox->currentText();
        });

        AbstractWidget *widget = new AbstractWidget(comboBox);
        windowService->addToolBarWidgetItem(buildTypeKey, widget);

        QAction *actionRun = new QAction(tr("Run"));
        ActionManager::getInstance()->registerAction(actionRun, "Build.Run", tr("Run"),
                                                     QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
                                                     "run-build-file.png");
        windowService->addToolBarActionItem(runKey, actionRun);
    } else {
        windowService->removeToolBarItem(buildTypeKey);
        windowService->removeToolBarItem(runKey);
    }
}

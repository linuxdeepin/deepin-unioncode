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
#include "mavenopenhandler.h"
#include "mavengenerator.h"
#include "transceiver/sendevents.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/toolchecker/toolcheckerservice.h"

#include "base/abstractaction.h"

#include <QFileDialog>

namespace {
CmakeOpenHandler *ins{ nullptr };
}

CmakeOpenHandler::CmakeOpenHandler(QObject *parent) : QObject(parent)
{

}

CmakeOpenHandler *CmakeOpenHandler::instance()
{
    if (!ins)
        ins = new CmakeOpenHandler;
    return ins;
}

QAction *CmakeOpenHandler::openAction()
{
    auto result = new QAction(MavenGenerator::toolKitName());
    QObject::connect(result, &QAction::triggered, [=](){

        QString iniPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("setting.ini");
        QSettings setting(iniPath, QSettings::IniFormat);
        QString lastPath = setting.value("mavenlastpath").toString();

        QFileDialog fileDialog;
        QString filePath = fileDialog.getOpenFileName(nullptr, "Open Maven Project", lastPath, "pom.xml");
        if(!filePath.isEmpty()) {

            // save open history
            QString lastPath = filePath.left(filePath.lastIndexOf('/'));
            QVariant varPath(lastPath);
            setting.setValue("mavenlastpath", varPath);

            QString kitName = result->text();
            QString language = dpfservice::MWMFA_JAVA;
            doProjectOpen(kitName, language, filePath);
            SendEvents::menuOpenProject(filePath, kitName, language); // 发送打开事件
        }
    });
    return result;
}

void CmakeOpenHandler::doProjectOpen(const QString &name, const QString &language, const QString &filePath)
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

    auto generator = projectService->createGenerator<MavenGenerator>(name);
    if (!generator)
        return;

    auto widget = generator->configureWidget(language, filePath);
    if (widget) {
        widget->setWindowFlag(Qt::Dialog);
        widget->show();
    }
}

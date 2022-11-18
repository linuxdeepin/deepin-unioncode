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
#include "pythonopenhandler.h"
#include "pythongenerator.h"
#include "transceiver/sendevents.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include "base/abstractaction.h"

#include <QFileDialog>

namespace {
PythonOpenHandler *ins{ nullptr };
}

PythonOpenHandler::PythonOpenHandler(QObject *parent) : QObject(parent)
{

}

PythonOpenHandler *PythonOpenHandler::instance()
{
    if (!ins)
        ins = new PythonOpenHandler;
    return ins;
}

QAction *PythonOpenHandler::openAction()
{
    auto result = new QAction(PythonGenerator::toolKitName());
    QObject::connect(result, &QAction::triggered, [=](){

        QString iniPath = CustomPaths::user(CustomPaths::Flags::Configures) +
                QDir::separator() + QString("setting.ini");
        QSettings setting(iniPath, QSettings::IniFormat);
        QString lastPath = setting.value("pythonlastpath").toString();

        QFileDialog fileDialog;
        QString filePath = fileDialog.getExistingDirectory(nullptr, "Open Python Project Directory",
                                                           lastPath, QFileDialog::DontResolveSymlinks);
        if(!filePath.isEmpty()) {

            // save open history
            QString lastPath = filePath.left(filePath.lastIndexOf('/'));
            QVariant varPath(lastPath);
            setting.setValue("pythonlastpath", varPath);

            QString kitName = result->text();
            QString language = dpfservice::MWMFA_PYTHON;
            doProjectOpen(kitName, language, filePath);
        }
    });
    return result;
}

void PythonOpenHandler::doProjectOpen(const QString &name, const QString &language, const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!projectService || !windowService)
        return;

    auto generator = projectService->createGenerator<PythonGenerator>(name);
    if (!generator)
        return;

    auto widget = generator->configureWidget(language, filePath);
    if (widget) {
        widget->setWindowFlag(Qt::Dialog);
        widget->show();
    }
}

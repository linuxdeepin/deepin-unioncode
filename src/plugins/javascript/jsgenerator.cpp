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

#include "jsgenerator.h"
#include "services/project/projectservice.h"
#include "common/widget/outputpane.h"
#include <framework/framework.h>

#include <QFile>
#include <QDBusMessage>
#include <QDBusConnection>

using namespace dpfservice;

class JSGeneratorPrivate
{
    friend class JSGenerator;
};

JSGenerator::JSGenerator()
    : d(new JSGeneratorPrivate())
{
}

JSGenerator::~JSGenerator()
{
    if (d)
        delete d;
}

QString JSGenerator::debugger()
{
    return "local";
}

bool JSGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    Q_UNUSED(param)
    Q_UNUSED(retMsg)
    return true;
}

bool JSGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();
    QString kit = "jsdirectory";

    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "getDebugPort");

    msg << uuid
        << kit
        << targetPath
        << arguments;
    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        retMsg = tr("Request debug dap port failed, please retry.");
        return false;
    }

    return true;
}

bool JSGenerator::isNeedBuild()
{
    return false;
}

bool JSGenerator::isTargetReady()
{
    return true;
}

bool JSGenerator::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest JSGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();

    dap::LaunchRequest request;
    request.name = "(jsdbg) Launch";
    request.type = "jsdbg";
    request.request = "launch";
    request.program = targetPath.toStdString();
    request.stopAtEntry = false;
    dap::array<dap::string> arrayArg;
    foreach (QString arg, arguments) {
        arrayArg.push_back(arg.toStdString());
    }
    request.args = arrayArg;
    request.externalConsole = false;
    request.MIMode = "jsdbg";
    request.__sessionId = QUuid::createUuid().toString().toStdString();

    return request;
}

QString JSGenerator::build(const QString& projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QString JSGenerator::getProjectFile(const QString& projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QMap<QString, QVariant> JSGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                          const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

RunCommandInfo JSGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(projectInfo)
    if (!QFile::exists(currentFile)) {
        OutputPane::instance()->appendText(tr("Please open a JS file in editor!"), OutputPane::ErrorMessage);
    }
    return {"node", {currentFile}, ""};
}

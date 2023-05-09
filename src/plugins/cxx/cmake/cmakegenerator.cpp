/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "cmakegenerator.h"

#include "cmakebuild.h"
#include "cmakedebug.h"
#include "services/project/projectservice.h"
#include <framework/framework.h>

#include <QFile>

using namespace dpfservice;

class CMakeGeneratorPrivate
{
    friend class CMakeGenerator;
    QSharedPointer<CMakeDebug> cmakeDebug;
};

CMakeGenerator::CMakeGenerator()
    : d(new CMakeGeneratorPrivate())
{
    d->cmakeDebug.reset(new CMakeDebug());
}

CMakeGenerator::~CMakeGenerator()
{
    if (d)
        delete d;
}

QString CMakeGenerator::debugger()
{
    return "dap";
}

bool CMakeGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    Q_UNUSED(param)
    return d->cmakeDebug->prepareDebug(retMsg);
}

bool CMakeGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();

    return d->cmakeDebug->requestDAPPort(uuid, toolKitName(), targetPath, arguments, retMsg);
}

bool CMakeGenerator::isNeedBuild()
{
    return true;
}

bool CMakeGenerator::isTargetReady()
{
    return true;
}

bool CMakeGenerator::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest CMakeGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();

    return d->cmakeDebug->launchDAP(targetPath, arguments);
}

QString CMakeGenerator::build(const QString& projectPath)
{
    return CMakeBuild::build(toolKitName(), projectPath);
}

QString CMakeGenerator::getProjectFile(const QString& projectPath)
{
    return projectPath + QDir::separator() + "CMakeList.txt";
}

QMap<QString, QVariant> CMakeGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                          const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.runWorkspaceDir());
    param.insert("targetPath", projectInfo.runProgram());

    return param;
}

RunCommandInfo CMakeGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)

    RunCommandInfo runCommaindInfo;
    runCommaindInfo.program = projectInfo.runProgram();
    runCommaindInfo.arguments = projectInfo.runCustomArgs();
    runCommaindInfo.workingDir = projectInfo.runWorkspaceDir();
    return runCommaindInfo;
}

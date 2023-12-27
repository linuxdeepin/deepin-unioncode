// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    runCommaindInfo.envs = projectInfo.runEnvironment();
    return runCommaindInfo;
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjagenerator.h"

#include "ninjabuild.h"
#include "ninjadebug.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"

#include <QFile>

using namespace dpfservice;

class NinjaGeneratorPrivate
{
    friend class NinjaGenerator;
    QSharedPointer<NinjaDebug> ninjaDebug;
};

NinjaGenerator::NinjaGenerator()
    : d(new NinjaGeneratorPrivate())
{
    d->ninjaDebug.reset(new NinjaDebug());
}

NinjaGenerator::~NinjaGenerator()
{
    if (d)
        delete d;
}

QString NinjaGenerator::debugger()
{
    return "dap";
}

bool NinjaGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    Q_UNUSED(param)
    Q_UNUSED(retMsg)
    return true;
}

bool NinjaGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();

    return d->ninjaDebug->requestDAPPort(uuid, toolKitName(), targetPath, arguments, retMsg);
}

bool NinjaGenerator::isNeedBuild()
{
    return true;
}

bool NinjaGenerator::isTargetReady()
{
    return true;
}

bool NinjaGenerator::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest NinjaGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString targetPath = param.value("targetPath").toString();
    QStringList arguments = param.value("arguments").toStringList();

    return d->ninjaDebug->launchDAP(targetPath, arguments);
}

QString NinjaGenerator::build(const QString &projectPath)
{
    return NinjaBuild::build(toolKitName(), projectPath);
}

QMap<QString, QVariant> NinjaGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                          const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    QString workspace = projectInfo.workspaceFolder();
    param.insert("workspace", workspace);
    QString targetPath = workspace + QDir::separator() + QFileInfo(workspace).fileName();
    param.insert("targetPath", targetPath);

    return param;
}

RunCommandInfo NinjaGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)

    RunCommandInfo runCommandInfo;
    QString workspace = projectInfo.workspaceFolder();
    QString targetPath = workspace + QDir::separator() + QFileInfo(workspace).fileName();
    runCommandInfo.program = targetPath;
    runCommandInfo.arguments = projectInfo.runCustomArgs();
    runCommandInfo.workingDir = workspace;

    return runCommandInfo;
}

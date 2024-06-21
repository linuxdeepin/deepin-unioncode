// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llgenerator.h"
#include "common/widget/outputpane.h"

using namespace dpfservice;

LLGenerator::LLGenerator()
{
}

LLGenerator::~LLGenerator()
{
}

QString LLGenerator::debugger()
{
    return "";
}

bool LLGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    Q_UNUSED(param)
    Q_UNUSED(retMsg)
    return true;
}

bool LLGenerator::requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    retMsg = tr("debug of Linglong project is not supported");
    return false;
}

bool LLGenerator::isNeedBuild()
{
    return false;
}

bool LLGenerator::isTargetReady()
{
    return true;
}

bool LLGenerator::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest LLGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    dap::LaunchRequest request;
    return request;
}

QString LLGenerator::build(const QString &projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QString LLGenerator::getProjectFile(const QString &projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QMap<QString, QVariant> LLGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                       const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    //param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

RunCommandInfo LLGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)
    RunCommandInfo info;
    info.program = "ll-builder";
    info.arguments.append("run");
    info.workingDir = projectInfo.workspaceFolder();
    return info;
}

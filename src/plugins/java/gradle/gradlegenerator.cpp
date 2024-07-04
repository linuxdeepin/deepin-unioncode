// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradlegenerator.h"

#include "java/javadebug.h"
#include "gradle/gradlebuild.h"
#include "javautil.h"
#include "services/project/projectservice.h"

using namespace dpfservice;

class GradleGeneratorPrivate
{
    friend class GradleGenerator;
    QSharedPointer<JavaDebug> javaDebug;
};

GradleGenerator::GradleGenerator()
    : d(new GradleGeneratorPrivate())
{
    d->javaDebug.reset(new JavaDebug());
}

GradleGenerator::~GradleGenerator()
{
    if (d)
        delete d;
}

QString GradleGenerator::debugger()
{
    return "dap";
}

bool GradleGenerator::isNeedBuild()
{
    return true;
}

bool GradleGenerator::isTargetReady()
{
    return  true;
}

bool GradleGenerator::isAnsyPrepareDebug()
{
    return d->javaDebug->isAnsyPrepareDebug();
}

bool GradleGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->prepareDebug(projectPath, retMsg);
}

bool GradleGenerator::requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->requestDAPPort(ppid, toolKitName(),projectPath, retMsg);
}

bool GradleGenerator::isLaunchNotAttach()
{
    return d->javaDebug->isLaunchNotAttach();
}

dap::LaunchRequest GradleGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString workspace = param.value("workspace").toString();
    QString mainClass = param.value("mainClass").toString();
    ProjectService *projectService = dpfGetService(ProjectService);
    ProjectInfo projectInfo = projectService->getActiveProjectInfo();
    QString mainClassPath = JavaUtil::getMainClassPath(projectInfo.workspaceFolder());
    QString packageDirName = "main";
    if (mainClass.isEmpty()) {
        mainClass = JavaUtil::getMainClass(mainClassPath, packageDirName);
    }
    QString projectName = param.value("projectName").toString();
    QStringList classPaths = param.value("classPaths").toStringList();
    if (classPaths.isEmpty()) {
        classPaths << JavaUtil::getPackageDir(mainClassPath, packageDirName);
    }
    return d->javaDebug->launchDAP(workspace, mainClass, projectName, classPaths);
}

bool GradleGenerator::isRestartDAPManually()
{
    return d->javaDebug->isRestartDAPManually();
}

bool GradleGenerator::isStopDAPManually()
{
    return d->javaDebug->isStopDAPManually();
}

QString GradleGenerator::build(const QString& projectPath)
{
    return GradleBuild::build(toolKitName(), projectPath);
}

QMap<QString, QVariant> GradleGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                           const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

dpfservice::RunCommandInfo GradleGenerator::getRunArguments(const dpfservice::ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)

    RunCommandInfo runCommandInfo;
    QString packageDirName = "main";

    QString mainClassPath = JavaUtil::getMainClassPath(projectInfo.workspaceFolder());
    runCommandInfo.program = "java";
    runCommandInfo.arguments << JavaUtil::getMainClass(mainClassPath, packageDirName);
    runCommandInfo.workingDir = JavaUtil::getPackageDir(mainClassPath, packageDirName);
    runCommandInfo.runInTerminal = false; // todo : config "run in terminal"

    return runCommandInfo;
}


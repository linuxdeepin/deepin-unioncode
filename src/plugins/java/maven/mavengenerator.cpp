// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavengenerator.h"
#include "java/javadebug.h"
#include "maven/mavenbuild.h"
#include "javautil.h"

#include <QRegularExpression>

using namespace dpfservice;

class MavenGeneratorPrivate
{
    friend class MavenGenerator;
    QSharedPointer<JavaDebug> javaDebug;
};

MavenGenerator::MavenGenerator()
    : d(new MavenGeneratorPrivate())
{
    d->javaDebug.reset(new JavaDebug());
}

MavenGenerator::~MavenGenerator()
{
    if (d)
        delete d;
}

QString MavenGenerator::debugger()
{
    return "dap";
}

bool MavenGenerator::isNeedBuild()
{
    return true;
}

bool MavenGenerator::isTargetReady()
{
    return true;
}

bool MavenGenerator::isAnsyPrepareDebug()
{
    return d->javaDebug->isAnsyPrepareDebug();
}

bool MavenGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->prepareDebug(projectPath, retMsg);
}

bool MavenGenerator::requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->requestDAPPort(ppid, toolKitName(), projectPath, retMsg);
}

bool MavenGenerator::isLaunchNotAttach()
{
    return d->javaDebug->isLaunchNotAttach();
}

dap::LaunchRequest MavenGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString workspace = param.value("workspace").toString();
    QString mainClass = param.value("mainClass").toString();
    QString projectName = param.value("projectName").toString();
    QStringList classPaths = param.value("classPaths").toStringList();

    return d->javaDebug->launchDAP(workspace, mainClass, projectName, classPaths);
}

bool MavenGenerator::isRestartDAPManually()
{
    return d->javaDebug->isRestartDAPManually();
}

bool MavenGenerator::isStopDAPManually()
{
    return d->javaDebug->isStopDAPManually();
}

QString MavenGenerator::build(const QString &projectPath)
{
    return MavenBuild::build(toolKitName(), projectPath);
}

QString MavenGenerator::getProjectFile(const QString &projectPath)
{
    return projectPath + QDir::separator() + "pom.xml";
}

QMap<QString, QVariant> MavenGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                          const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

RunCommandInfo MavenGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)

    RunCommandInfo runCommandInfo;
    QString packageDirName = "classes";

    QString mainClassPath = JavaUtil::getMainClassPath(projectInfo.workspaceFolder());
    runCommandInfo.program = "java";
    runCommandInfo.arguments << JavaUtil::getMainClass(mainClassPath, packageDirName);
    runCommandInfo.workingDir = JavaUtil::getPackageDir(mainClassPath, packageDirName);
    runCommandInfo.runInTerminal = false; // todo : config "run in terminal"

    return runCommandInfo;
}

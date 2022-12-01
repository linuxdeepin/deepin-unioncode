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
#include "mavengenerator.h"

#include "java/javadebug.h"
#include "maven/mavenbuild.h"

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

bool MavenGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->requestDAPPort(uuid, toolKitName(), projectPath, retMsg);
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

QString MavenGenerator::build(const QString& projectPath)
{
    return MavenBuild::build(toolKitName(), projectPath);
}

QString MavenGenerator::getProjectFile(const QString& projectPath)
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

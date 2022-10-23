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

bool MavenGenerator::prepareDebug(const QString &projectPath, const QString &fileName, QString &retMsg)
{
    Q_UNUSED(fileName)
    return d->javaDebug->prepareDebug(projectPath, retMsg);
}

bool MavenGenerator::requestDAPPort(const QString &uuid, const QString &projectPath, const QString &fileName, QString &retMsg)
{
    Q_UNUSED(fileName)
    return d->javaDebug->requestDAPPort(uuid, projectPath, retMsg);
}

bool MavenGenerator::isLaunchNotAttach()
{
    return d->javaDebug->isLaunchNotAttach();
}

dap::LaunchRequest MavenGenerator::launchDAP(int port,
                                             const QString &workspace,
                                             const QString &mainClass,
                                             const QString &projectName,
                                             const QStringList &classPaths)
{
    Q_UNUSED(port)
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

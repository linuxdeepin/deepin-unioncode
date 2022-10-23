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

bool CMakeGenerator::prepareDebug(const QString &projectPath, const QString &fileName, QString &retMsg)
{
    Q_UNUSED(projectPath)
    Q_UNUSED(fileName)
    Q_UNUSED(retMsg)
    return d->cmakeDebug->prepareDebug();
}

bool CMakeGenerator::requestDAPPort(const QString &uuid, const QString &projectPath, const QString &fileName, QString &retMsg)
{
    Q_UNUSED(projectPath)
    Q_UNUSED(fileName)
    return d->cmakeDebug->requestDAPPort(uuid, retMsg);
}

bool CMakeGenerator::isNeedBuild()
{
    return true;
}

bool CMakeGenerator::isTargetReady()
{
    QString targetPath = CMakeBuild::getTargetPath();
    if (targetPath.isEmpty())
        return false;

    return QFile::exists(targetPath);
}

dap::LaunchRequest CMakeGenerator::launchDAP(int port,
                                             const QString &workspace,
                                             const QString &mainClass,
                                             const QString &projectName,
                                             const QStringList &classPaths)
{
    Q_UNUSED(port)
    Q_UNUSED(workspace)
    Q_UNUSED(mainClass)
    Q_UNUSED(projectName)
    Q_UNUSED(classPaths)

    QString targetPath = CMakeBuild::getTargetPath();
    return d->cmakeDebug->launchDAP(targetPath);
}

QString CMakeGenerator::build(const QString& projectPath)
{
    return CMakeBuild::build(toolKitName(), projectPath);
}


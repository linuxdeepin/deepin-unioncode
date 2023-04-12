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
#ifndef LANGUAGEGENERATOR_H
#define LANGUAGEGENERATOR_H

#include "common/common.h"
#include "dap/protocol.h"
#include "services/project/projectinfo.h"

namespace dpfservice {

struct RunCommandInfo {
    QString program;
    QStringList arguments;
    QString workingDir;
};

class LanguageGenerator : public Generator
{
    Q_OBJECT
public:
    LanguageGenerator(){}
    virtual ~LanguageGenerator(){}

    virtual bool isNeedBuild() {
        return false;
    }

    virtual bool isTargetReady() {
        return false;
    }

    virtual bool isAnsyPrepareDebug() {
        return false;
    }

    virtual QString debugger() = 0;
    virtual bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) = 0;
    virtual bool requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg) = 0;
    virtual bool isLaunchNotAttach() = 0;

    virtual dap::LaunchRequest launchDAP(const QMap<QString, QVariant> &param) {
        Q_UNUSED(param)
        return dap::LaunchRequest();
    }

    virtual dap::AttachRequest attachDAP(int port, const QMap<QString, QVariant> &param) {
        Q_UNUSED(port)
        Q_UNUSED(param)
        return dap::AttachRequest();
    }

    virtual bool isRestartDAPManually() {
        return false;
    }

    virtual bool isStopDAPManually() {
        return false;
    }

    virtual QString build(const QString& projectPath) {
        Q_UNUSED(projectPath)
        return QString();
    }

    virtual QString getProjectFile(const QString& projectPath) {
        return projectPath;
    }

    virtual QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                      const QString &currentFile) {
        Q_UNUSED(projectInfo)
        Q_UNUSED(currentFile)
        return QMap<QString, QVariant>();
    }

    virtual RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                                      const QString &currentFile) {
        Q_UNUSED(projectInfo)
        Q_UNUSED(currentFile)
        return {};
    }
};

} // namespace dpfservice

#endif // LANGUAGEGENERATOR_H

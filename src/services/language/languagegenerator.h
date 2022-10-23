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

namespace dpfservice {
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

    virtual bool prepareDebug(const QString &projectPath,
                              const QString &fileName,
                              QString &retMsg) {
        Q_UNUSED(projectPath)
        Q_UNUSED(fileName)
        Q_UNUSED(retMsg)
        return true;
    }

    virtual bool requestDAPPort(const QString &uuid,
                              const QString &projectPath,
                              const QString &fileName,
                              QString &retMsg) {
        Q_UNUSED(uuid)
        Q_UNUSED(projectPath)
        Q_UNUSED(fileName)
        Q_UNUSED(retMsg)
        return true;
    }

    virtual bool isLaunchNotAttach() {
        return true;
    }

    virtual dap::LaunchRequest launchDAP(int port,
                                         const QString &workspace,
                                         const QString &mainClass,
                                         const QString &projectName,
                                         const QStringList &classPaths) {
        Q_UNUSED(port)
        Q_UNUSED(workspace)
        Q_UNUSED(mainClass)
        Q_UNUSED(projectName)
        Q_UNUSED(classPaths)
        return dap::LaunchRequest();
    }

    virtual dap::AttachRequest attachDAP(int port, const QString &workspace) {
        Q_UNUSED(port)
        Q_UNUSED(workspace)
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
};

} // namespace dpfservice

#endif // LANGUAGEGENERATOR_H

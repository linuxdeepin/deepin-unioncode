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
#ifndef MAVENGENERATOR_H
#define MAVENGENERATOR_H

#include "services/language/languagegenerator.h"
#include "dap/protocol.h"

class MavenGeneratorPrivate;
class MavenGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit MavenGenerator();
    virtual ~MavenGenerator() override;

    static QString toolKitName() { return "maven"; }

    bool isNeedBuild() override;
    bool isTargetReady() override;
    bool isAnsyPrepareDebug() override;
    bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool isLaunchNotAttach() override;
    dap::LaunchRequest launchDAP(const QMap<QString, QVariant> &param) override;
    QString build(const QString& projectPath) override;
    QString getProjectFile(const QString& projectPath) override;
    bool isRestartDAPManually() override;
    bool isStopDAPManually() override;
    QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                              const QString &currentFile) override;
    virtual dpfservice::RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                                      const QString &currentFile);
signals:

private slots:

private:
    MavenGeneratorPrivate *const d;
};

#endif // MAVENGENERATOR_H

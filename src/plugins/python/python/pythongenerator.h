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
#ifndef PYTHONGENERATOR_H
#define PYTHONGENERATOR_H

#include "services/language/languagegenerator.h"
#include "dap/protocol.h"

class PythonGeneratorPrivate;
class PythonGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit PythonGenerator();
    virtual ~PythonGenerator() override;

    static QString toolKitName() { return "directory"; }

    bool isTargetReady() override;
    bool prepareDebug(const QString &projectPath,
                      const QString &fileName,
                      QString &retMsg) override;
    bool requestDAPPort(const QString &uuid,
                        const QString &projectPath,
                        const QString &fileName,
                        QString &retMsg) override;
    bool isLaunchNotAttach() override;
    dap::AttachRequest attachDAP(int port, const QString &workspace) override;
    bool isRestartDAPManually() override;
    bool isStopDAPManually() override;

signals:

private slots:

private:
    PythonGeneratorPrivate *const d;
};

#endif // PYTHONGENERATOR_H

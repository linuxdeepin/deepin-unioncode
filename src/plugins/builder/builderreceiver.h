/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef BUILDERRECEIVER_H
#define BUILDERRECEIVER_H

#include <framework/framework.h>
#include "builderglobals.h"

/**
 * @brief This is a tempory class for provide project info.
 * TODO(mozart) : delete this class
 */
class BuilderReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<BuilderReceiver>
{
    friend class dpf::AutoEventHandlerRegister<BuilderReceiver>;
public:
    static BuilderReceiver *instance();
    static Type type();
    static QStringList &topics();

    const QString &projectFilePath() const;
    const QString &projectDirectory() const;
    const QString &rootProjectDirectory() const;
    const QString &buildOutputDirectory() const;

    ToolChainType toolChainType() const;

private:
    explicit BuilderReceiver(QObject * parent = nullptr);

    virtual void eventProcess(const dpf::Event& event) override;
    void updatePaths(const QString &path);

    QString proFilePath;
    QString proDirPath;
    QString proRootPath;
    QString buildOutputPath;

    ToolChainType tlChainType = ToolChainType::UnKnown;
};

#endif // BUILDERRECEIVER_H

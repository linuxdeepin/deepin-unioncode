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
#ifndef BUILDERSENDER_H
#define BUILDERSENDER_H

#include "services/builder/builderglobals.h"

#include <QObject>

class BuilderSender final : QObject
{
    Q_OBJECT
public:
    static void sendCommand(const QString &program, const QStringList &arguments, const QString &workingDir);
    static void menuBuild();
    static void menuReBuild();
    static void menuClean();
    static void jumpTo(const QString &filePath, int lineNum);
    static void notifyBuildState(BuildState state, QString originCmd);

private:
    explicit BuilderSender(QObject *parent = nullptr);
    virtual ~BuilderSender();
};

#endif   // EVENTSENDER_H

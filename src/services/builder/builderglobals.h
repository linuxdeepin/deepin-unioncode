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
#ifndef BUILDERGLOBALS_H
#define BUILDERGLOBALS_H

#include <common/util/singleton.h>
#include <QMetaType>
#include <QColor>
#include <QUuid>

enum ToolChainType {
    UnKnown,
    QMake,
    CMake
};

enum BuildState
{
    kNoBuild,
    kBuilding,
    kBuildFailed
};

enum BuildMenuType
{
    Build = 0,
    Clean
};

struct BuildCommandInfo {
    QString kitName;
    QString program;
    QStringList arguments;
    QString workingDir;
    QString uuid;

    BuildCommandInfo() {
        uuid = QUuid::createUuid().toString();
    }
};

Q_DECLARE_METATYPE(BuildCommandInfo);



#endif // BUILDERGLOBALS_H

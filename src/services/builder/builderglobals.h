// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QString elfPath;

    BuildCommandInfo() {
        uuid = QUuid::createUuid().toString();
    }
};

Q_DECLARE_METATYPE(BuildCommandInfo);



#endif // BUILDERGLOBALS_H

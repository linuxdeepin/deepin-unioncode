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
#include <QColor>

enum ToolChainType {
    UnKnown,
    QMake,
    CMake
};

enum OutputFormat {
    Stdout,
    Stderr,
    NormalMessage,
    ErrorMessage
};

enum BuildState
{
    kNoBuild,
    kBuilding,
    kBuildFailed
};


/**
 * @brief Output text color.
 */
const QColor kTextColorNormal(150, 150, 150);
const QColor kErrorMessageTextColor(255, 108, 108);
const QColor kMessageOutput(0, 135, 135);

#endif // BUILDERGLOBALS_H

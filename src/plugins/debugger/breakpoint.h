/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include <dap/protocol.h>
#include <QString>

namespace Internal
{

enum BreakpointType
{
    UnknownBreakpointType,
    BreakpointByFileAndLine,
    BreakpointByFunction,
    BreakpointByAddress,
    BreakpointAtThrow,
    BreakpointAtCatch,
    BreakpointAtExec,
    LastBreakpointType
};

class Breakpoint
{
public:
    Breakpoint();

    void update(dap::Breakpoint &bp);

    bool operator==(const Breakpoint& o) const
    {
        if (filePath == o.filePath
                && lineNumber == o.lineNumber)
            return true;
        return false;
    }

    bool enabled = true;
    QString fileName;
    QString filePath;
    int lineNumber = 0;
    int threadSpec = 0;
    QString functionName;
    QString module;
    BreakpointType type = BreakpointByFileAndLine;
    QString address;
};

using Breakpoints = QVector<Breakpoint>;
}

#endif // BREAKPOINT_H

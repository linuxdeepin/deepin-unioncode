// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QString condition;
};

using Breakpoints = QVector<Breakpoint>;
}

#endif // BREAKPOINT_H

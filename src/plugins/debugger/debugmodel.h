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
#ifndef DEBUGMODEL_H
#define DEBUGMODEL_H

#include <QObject>
#include "debug.h"
#include "debugsession.h"

class DebugModel : public QObject, IDebugModel
{
    Q_OBJECT
public:
    explicit DebugModel(DebugSession *session, QObject *parent = nullptr);

//    IDebugSession getSessions(bool includeInactive);
    ReadonlyArray<IBreakpoint> getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber, dap::optional<int> column, dap::optional<bool> enabledOnly);
    bool areBreakpointsActivated();
    ReadonlyArray<IFunctionBreakpoint> getFunctionBreakpoints();
    ReadonlyArray<IDataBreakpoint> getDataBreakpoints();
    ReadonlyArray<IExceptionBreakpoint> getExceptionBreakpoints();
    ReadonlyArray<IInstructionBreakpoint> getInstructionBreakpoints();

    ReadonlyArray<IBreakpoint> addBreakpoints(QUrl &uri, dap::array<IBreakpoint> &rawData, bool fireEvent = true);

    void setBreakpointsActivated(bool activated);

signals:

public slots:

private:
    bool breakpointsActivated = true;
    dap::array<IBreakpoint> breakPoints;
    dap::array<IFunctionBreakpoint> functionBreakpoints;
    dap::array<IExceptionBreakpoint> exceptionBreakpoints;
    dap::array<IDataBreakpoint> dataBreakpoints;
    dap::array<IInstructionBreakpoint> instructionBreakpoints;

    DebugSession *session = nullptr;
};

#endif // DEBUGMODEL_H

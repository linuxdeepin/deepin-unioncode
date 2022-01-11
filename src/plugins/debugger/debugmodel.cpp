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
#include "debugmodel.h"

using namespace dap;
DebugModel::DebugModel(DebugSession *_session, QObject *parent) :
    QObject(parent),
    session(_session)
{

}

dap::array<IBreakpoint> DebugModel::getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber, dap::optional<int> column, dap::optional<bool> enabledOnly)
{
    dap::array<IBreakpoint> ret;
    string uriStr = url ? url->toString().toStdString() : "";
    for (auto it : breakPoints) {
        if ((url && it.uri.toString().toStdString() != uriStr)
                || (lineNumber && lineNumber.value() != it.lineNumber)
                || (enabledOnly && (!breakpointsActivated || enabledOnly.value() != it.enabled))) {
            continue;
        }
        ret.push_back(it);
    }
    return ret;
}

bool DebugModel::areBreakpointsActivated()
{
    return breakpointsActivated;
}

dap::array<IFunctionBreakpoint> DebugModel::getFunctionBreakpoints()
{
    return functionBreakpoints;
}

dap::array<IDataBreakpoint> DebugModel::getDataBreakpoints()
{
    return dataBreakpoints;
}

dap::array<IExceptionBreakpoint> DebugModel::getExceptionBreakpoints()
{
    return exceptionBreakpoints;
}

dap::array<IInstructionBreakpoint> DebugModel::getInstructionBreakpoints()
{
    return instructionBreakpoints;
}

dap::array<IBreakpoint> DebugModel::addBreakpoints(QUrl &uri, dap::array<IBreakpoint> &rawData, bool fireEvent)
{
    Q_UNUSED(uri)
    Q_UNUSED(fireEvent)
    breakPoints.insert(breakPoints.end(), rawData.begin(), rawData.end());
    return breakPoints;
}

void DebugModel::setBreakpointsActivated(bool activated)
{
    breakpointsActivated = activated;
    // fire event.
}

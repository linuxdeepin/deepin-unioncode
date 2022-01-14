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

#include "debug.h"
#include "debugsession.h"

#include <QObject>

class DebugModel : public QObject, IDebugModel
{
    Q_OBJECT
public:
    explicit DebugModel(QObject *parent = nullptr);

    void clearThreads(dap::string id, bool removeThreads, dap::optional<number> reference);

    ReadonlyArray<IBreakpoint> getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber,
                                              dap::optional<int> column, dap::optional<bool> enabledOnly);
    ReadonlyArray<IFunctionBreakpoint> getFunctionBreakpoints();
    ReadonlyArray<IDataBreakpoint> getDataBreakpoints();
    ReadonlyArray<IExceptionBreakpoint> getExceptionBreakpoints();
    ReadonlyArray<IInstructionBreakpoint> getInstructionBreakpoints();

    bool areBreakpointsActivated();
    void setBreakpointsActivated(bool activated);
    ReadonlyArray<IBreakpoint> addBreakpoints(QUrl &uri, dap::array<IBreakpointData> &rawData, bool fireEvent = true);
    void removeBreakpoints(dap::array<IBreakpoint> &toRemove);
    void updateBreakpoints(std::map<dap::string, IBreakpointUpdateData> &data);
    void setBreakpointSessionData(dap::string &sessionId, dap::Capabilities &capabilites, dap::optional<std::map<dap::string, dap::Breakpoint>> data);
    void enableOrDisableAllBreakpoints(bool enable);
    IFunctionBreakpoint addFunctionBreakpoint(dap::string &functionName, dap::string &id);
    void updateFunctionBreakpoint(dap::string &id, dap::optional<dap::string> name, dap::optional<dap::string> hitCondition, dap::optional<dap::string> condition);
    void removeFunctionBreakpoints(dap::string &id);
    void addDataBreakpoint(dap::string &label, dap::string &dataId, bool canPersist, dap::optional<dap::array<dap::DataBreakpointAccessType>> accessTypes, dap::DataBreakpointAccessType accessType);
    void removeDataBreakpoints(dap::string &id);
#if 0
    void addInstructionBreakpoint(dap::string address, number offset, dap::optional<dap::string> condition, dap::optional<dap::string> hitCondition);
    void removeInstructionBreakpoints(dap::optional<dap::string> address);
    dap::array<Expression> getWatchExpressions();
    IExpression addWatchExpression(dap::optional<dap::string> name);
    void renameWatchExpression(dap::string &id, dap::string &newName);
    void removeWatchExpressions(dap::string &id);
    void moveWatchExpression(dap::string id, number position);
#endif
    void sourceIsNotAvailable(QUrl uri);
signals:

public slots:

private:
    bool breakpointsActivated = true;
    dap::array<Breakpoint> breakPoints;
    dap::array<IFunctionBreakpoint> functionBreakpoints;
    dap::array<IExceptionBreakpoint> exceptionBreakpoints;
    dap::array<IDataBreakpoint> dataBreakpoints;
    dap::array<IInstructionBreakpoint> instructionBreakpoints;
};

#endif   // DEBUGMODEL_H

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
#ifndef DEBUG_H
#define DEBUG_H

#include "dap/protocol.h"

#include <QString>
#include <QUrl>

//#define string QString
#define number int

#define ReadonlyArray dap::array

struct IBreakpointData {
    QString id;
    number lineNumber;
    number column;
    bool enabled;
    QString condition;
    QString logMessage;
    QString hitCondition;
    QUrl source;
};

struct IBreakpointUpdateData {
    QString condition;
    QString hitCondition;
    QString logMessage;
    number lineNumber;
    number column;
};

struct ITreeElement {
    QString getId();
};

struct IEnablement : public ITreeElement {
    bool enabled;
};

struct IBaseBreakpoint : public IEnablement {
    QString condition;
    QString hitCondition;
    QString logMessage;
    bool verified;
    bool supported;
    QString message;
//    string sessionsThatVerified[];
    int getIdFromAdapter(QString sessionId);
};

struct IBreakpoint : public IBaseBreakpoint {
    QUrl uri;
    number lineNumber;
    number endLineNumber;
    number column;
    number endColumn;

    IBreakpoint(QUrl _url,
                number _lineNumber,
                number _endLineNumber,
                number _column,
                number _endColumn) :
        uri(_url),
        lineNumber(_lineNumber),
        endLineNumber(_endLineNumber),
        column(_column),
        endColumn(_endColumn)
    {
    }
};

struct IFunctionBreakpoint : public IBaseBreakpoint {
    QString name;
};

struct IExceptionBreakpoint : public IBaseBreakpoint {
    QString filter;
    QString label;
    QString description;
};

struct IDataBreakpoint : IBaseBreakpoint {
    QString description;
    QString dataId;
    bool canPersist;
    dap::DataBreakpointAccessType accessType;
};

struct IInstructionBreakpoint : public IBaseBreakpoint {
    // instructionReference is the instruction 'address' from the debugger.
    QString instructionReference;
    number offset;
};

struct IExceptionInfo {
    QString id;
    QString description;
    QString breakMode;
    dap::ExceptionDetails details;
};

struct IDebugSession : public ITreeElement {

};

struct IDebugModel : public ITreeElement {
    IDebugSession getSession(QString sessionId, bool includeInactive);
//    IDebugSession getSessions(bool includeInactive);
    ReadonlyArray<IBreakpoint> getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber, dap::optional<int> column, dap::optional<bool> enabledOnly);
    bool areBreakpointsActivated();
    ReadonlyArray<IFunctionBreakpoint> getFunctionBreakpoints();
    ReadonlyArray<IDataBreakpoint> getDataBreakpoints();
    ReadonlyArray<IExceptionBreakpoint> getExceptionBreakpoints();
    ReadonlyArray<IInstructionBreakpoint> getInstructionBreakpoints();
#if 0 // No need to use those now
    ReadonlyArray<IExpression & IEvaluate> getWatchExpressions();

    struct IBreakpointsChangeEvent;
    using BPChangeHandler = void(*)(IBreakpointsChangeEvent);
    void onDidChangeBreakpoints(BPChangeHandler handlder);
    using CBChangeHandler = void(*)(void);
    void onDidChangeCallStack(CBChangeHandler handler);
    struct IExpression;
    using WatchChangeHandler = void(*)(IExpression);
#endif
};


struct IRawStoppedDetails {
    dap::string reason;
    dap::string description;
    number threadId;
    dap::string text;
    number totalFrames;
    bool allThreadsStopped;
    dap::string framesErrorMessage;
    dap::array<number> hitBreakpointIds;
};

#endif // DEBUG_H

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
#include <QUuid>

#include <map>

#define Map map
#define undefined \
    {             \
    }
#define number dap::integer
#define ReadonlyArray dap::array

struct IBreakpointData
{
    dap::optional<dap::string> id;
    dap::optional<number> lineNumber;
    dap::optional<number> column;
    bool enabled;
    dap::optional<dap::string> condition;
    dap::optional<dap::string> logMessage;
    dap::optional<dap::string> hitCondition;
};

struct IBreakpointUpdateData
{
    dap::optional<dap::string> condition;
    dap::optional<dap::string> hitCondition;
    dap::optional<dap::string> logMessage;
    dap::optional<number> lineNumber;
    dap::optional<number> column;
};

struct ITreeElement
{
    dap::string getId();
};

struct IEnablement : public ITreeElement
{
    bool enabled;
};

struct IBaseBreakpoint : public IEnablement
{
    dap::optional<dap::string> condition;
    dap::optional<dap::string> hitCondition;
    dap::optional<dap::string> logMessage;
    bool verified;
    bool support; // suported.
    dap::optional<dap::string> message;
    dap::array<dap::string> sessionsThatVerified;
    dap::optional<number> getIdFromAdapter(dap::string sessionId);
};

struct IInnerBreakpoint
{
    QUrl uri;
    number lineNumber;
    dap::optional<number> endLineNumber;
    dap::optional<number> column;
    dap::optional<number> endColumn;
    dap::any adapterData;
};

struct IBreakpoint : public IBaseBreakpoint, public IInnerBreakpoint
{
};

struct IFunctionBreakpoint : public IBaseBreakpoint
{
    QString name;
};

struct IExceptionBreakpoint : public IBaseBreakpoint
{
    QString filter;
    QString label;
    QString description;
};

struct IDataBreakpoint : IBaseBreakpoint
{
    QString description;
    QString dataId;
    bool canPersist;
    dap::DataBreakpointAccessType accessType;
};

struct IInstructionBreakpoint : public IBaseBreakpoint
{
    // instructionReference is the instruction 'address' from the debugger.
    QString instructionReference;
    number offset;
};

struct IExceptionInfo
{
    QString id;
    QString description;
    QString breakMode;
    dap::ExceptionDetails details;
};

struct IDebugSession : public ITreeElement
{
};

struct IDebugModel : public ITreeElement
{
    IDebugSession getSession(QString sessionId, bool includeInactive);
    //    IDebugSession getSessions(bool includeInactive);
    ReadonlyArray<IBreakpoint> getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber, dap::optional<int> column, dap::optional<bool> enabledOnly);
    bool areBreakpointsActivated();
    ReadonlyArray<IFunctionBreakpoint> getFunctionBreakpoints();
    ReadonlyArray<IDataBreakpoint> getDataBreakpoints();
    ReadonlyArray<IExceptionBreakpoint> getExceptionBreakpoints();
    ReadonlyArray<IInstructionBreakpoint> getInstructionBreakpoints();
#if 0   // No need to use those now
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

struct IRawStoppedDetails
{
    dap::string reason;
    dap::string description;
    number threadId;
    dap::string text;
    number totalFrames;
    bool allThreadsStopped;
    dap::string framesErrorMessage;
    dap::array<number> hitBreakpointIds;
};

/**
  * Base structs.
  */

struct Enablement : public IEnablement
{
    Enablement(bool _enabled, dap::string &_id)
        : id(_id)
    {
        enabled = _enabled;
    }

    dap::string getId()
    {
        return id;
    }

private:
    dap::string id;
};

struct IBreakpointSessionData : public dap::Breakpoint
{
    bool supportsConditionalBreakpoints;
    bool supportsHitConditionalBreakpoints;
    bool supportsLogPoints;
    bool supportsFunctionBreakpoints;
    bool supportsDataBreakpoints;
    bool supportsInstructionBreakpoints;
    dap::string sessionId;
};

struct BaseBreakpoint : public IBaseBreakpoint
{
    BaseBreakpoint(
            bool _enabled,
            dap::optional<dap::string> _hitCondition,
            dap::optional<dap::string> _condition,
            dap::optional<dap::string> _logMessage,
            const dap::string &_id)
        : id(_id)
    {
        enabled = _enabled;
        hitCondition = _hitCondition;
        condition = _condition;
        logMessage = _logMessage;
    }

    virtual ~BaseBreakpoint() {}

    void setSessionData(dap::string &sessionId, dap::optional<IBreakpointSessionData> data)
    {
        if (!data) {
            auto it = sessionData.begin();
            for (; it != sessionData.end(); ++it) {
                if (sessionId == it->first) {
                    it = sessionData.erase(it);
                }
            }
        } else {
            data.value().sessionId = sessionId;
            sessionData.insert(std::pair<dap::string, IBreakpointSessionData>(sessionId, data.value()));
        }

        // TODO(mozart)
    }

    dap::optional<dap::string> message()
    {
        if (!data) {
            return undefined;
        }
        return data.value().message;
    }

    bool verified()
    {
        if (data) {
            return data.value().verified;
        }
        return true;
    }

    dap::array<dap::string> sessionsThatVerified()
    {
        dap::array<dap::string> sessionIds;

        auto it = sessionData.begin();
        for (; it != sessionData.end(); ++it) {
            if (it->second.verified) {
                sessionIds.push_back(it->first);
            }
        }
        return sessionIds;
    }

    virtual bool supported() = 0;

    dap::optional<number> getIdFromAdapter(dap::string &sessionId)
    {
        dap::optional<IBreakpointSessionData> data = getData(sessionId);
        if (data) {
            return data.value().id;
        }
        return undefined;
    }

    dap::optional<IBreakpointSessionData> getData(dap::string &sessionId)
    {
        auto it = sessionData.begin();
        bool bFound = false;
        for (; it != sessionData.end(); ++it) {
            if (it->first == sessionId) {
                data = it->second;
                bFound = true;
                break;
            }
        }
        if (bFound) {
            return data;
        }
        return undefined;
    }

    dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &sessionId)
    {
        dap::optional<dap::Breakpoint> bp;
        auto data = getData(sessionId);
        if (data) {
            bp.value().id = data->id;
            bp.value().verified = data->verified;
            bp.value().message = data->message;
            bp.value().source = data->source;
            bp.value().line = data->line;
            bp.value().column = data->column;
            bp.value().endLine = data->endLine;
            bp.value().endColumn = data->endColumn;
            bp.value().instructionReference = data->instructionReference;
            bp.value().offset = data->offset;
        }
        return bp;
    }

protected:
    dap::optional<IBreakpointSessionData> data;

    std::map<dap::string, IBreakpointSessionData> sessionData;
    dap::string id;
};

struct Breakpoint : public BaseBreakpoint, public IInnerBreakpoint
{
    Breakpoint(
            QUrl &uri,
            number lineNumber,
            dap::optional<number> column,
            bool enabled,
            dap::optional<dap::string> condition,
            dap::optional<dap::string> hitCondition,
            dap::optional<dap::string> logMessage,
            dap::any adapterData,
            const std::string &id = QUuid::createUuid().toString().toStdString())
        : BaseBreakpoint(enabled, hitCondition, condition, logMessage, id), _uri(uri), _lineNumber(lineNumber), _column(column), _adapterData(adapterData)
    {
    }

    bool isDirty(QUrl uri)
    {
        Q_UNUSED(uri)
        // Not support dirty check now.
        return false;
    }

    number lineNumber()
    {
        if (verified() && data && data.value().line) {
            return data.value().line.value();
        }
        return _lineNumber;
    }

    bool verified()
    {
        if (data) {
            return data.value().verified && !isDirty(_uri);
        }
        return true;
    }

    QUrl getUriFromSource(dap::Source source, dap::optional<dap::string> path, dap::string sessionId)
    {
        Q_UNUSED(path)
        Q_UNUSED(sessionId)
        return QUrl(source.path->c_str());
    }

    QUrl uri()
    {
        if (verified() && data && data.value().source) {
            return getUriFromSource(data.value().source.value(), data.value().source.value().path, data->sessionId);
        }
        return _uri;
    }

    dap::optional<dap::integer> column()
    {
        if (verified() && data && data.value().column) {
            return data.value().column;
        }
        return _column;
    }

    dap::optional<dap::string> message()
    {
        if (isDirty(uri())) {
            return "Unverified breakpoint. File is modified, please restart debug session.";
        }
        return BaseBreakpoint::message();
    }

    dap::any adapterData()
    {
        if (data && data.value().source && data.value().source.value().adapterData) {
            return data.value().source.value().adapterData.value();
        }
        return _adapterData;
    }

    dap::optional<number> endLineNumber()
    {
        if (verified() && data) {
            return data.value().endLine;
        }
        return undefined;
    }

    dap::optional<number> endColumn()
    {
        if (verified() && data) {
            return data.value().endColumn;
        }
        return undefined;
    }

    bool supported()
    {
        if (!data) {
            return true;
        }
        if (logMessage && !data.value().supportsLogPoints) {
            return false;
        }
        if (condition && !data.value().supportsConditionalBreakpoints) {
            return false;
        }
        if (hitCondition && !data.value().supportsHitConditionalBreakpoints) {
            return false;
        }

        return true;
    }

    void setSessionData(dap::string &sessionId, dap::optional<IBreakpointSessionData> data)
    {
        BaseBreakpoint::setSessionData(sessionId, data);
        if (_adapterData.is<std::nullptr_t>()) {
            _adapterData = adapterData();
        }
    }

    dap::string toString()
    {
        return undefined;
    }

    void update(IBreakpointUpdateData &data)
    {
        if (!data.lineNumber) {
            _lineNumber = data.lineNumber.value();
        }
        if (!data.column) {
            _column = data.column;
        }
        if (!data.condition) {
            condition = data.condition;
        }
        if (!data.hitCondition) {
            hitCondition = data.hitCondition;
        }
        if (!data.logMessage) {
            logMessage = data.logMessage;
        }
    }

private:
    QUrl _uri;
    number _lineNumber;
    dap::optional<number> _column;
    dap::any _adapterData;
};

#endif   // DEBUG_H

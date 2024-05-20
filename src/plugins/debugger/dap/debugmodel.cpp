// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugmodel.h"
#include "debug.h"
#include "debuggerglobals.h"

#include <QUuid>
#include <QString>

#include "stdlib.h"

namespace DEBUG_NAMESPACE {
DebugModel::DebugModel(dap::optional<dap::array<DebugSession *>> _sessions, QObject *parent)
    : QObject(parent)
{
    if (_sessions) {
        auto sessionArray = _sessions.value();
        sessions.insert(sessions.end(), sessionArray.begin(), sessionArray.end());
    }
}

dap::array<DebugSession *> DebugModel::getSessions(bool includeInactive)
{
    dap::array<DebugSession *> ret;

    std::copy_if(sessions.begin(), sessions.end(), std::back_inserter(ret), [&](const DebugSession *item){
         return (includeInactive || item->state != State::kInactive);
    });

    return ret;
}

dap::optional<DebugSession *> DebugModel::getSession(dap::optional<dap::string> sessionId, bool includeInactive)
{
    if (sessionId) {
        auto filteredSessions = getSessions(includeInactive);
        for (auto s : filteredSessions) {
            if (s->getId() == sessionId.value()) {
                return s;
            }
        }
    }
    return undefined;
}

void DebugModel::addSession(DebugSession *session)
{
    Q_ASSERT(session);
    dap::array<DebugSession *> filterdSessions;
    for (auto s = sessions.begin(); s != sessions.end();) {
        if ((*s)->getId() == session->getId()) {
            s = sessions.erase(s);
            return;
        }
        if ((*s)->state == State::kInactive && (*s)->configuration->name == session->configuration->name) {
            s = sessions.erase(s);
            return;
        }
        ++s;
    }
    int i = 1;
    char szBuf[10] = { 0 };
    for (auto s = sessions.begin(); s != sessions.end(); ++s) {
        while ((*s)->getLabel() == session->getLabel()) {
            sprintf(szBuf, "%d", i);
            auto newName = session->configuration->name + szBuf;
            session->setName(newName);
            memset(szBuf, 0, sizeof(szBuf));
        }
    }

    sessions.push_back(session);
}

void DebugModel::clear()
{
    sessions.clear();
}

void DebugModel::rawUpdate(IRawModelUpdate *data)
{
    for (auto it : sessions) {
        if (it->getId() == data->sessionId) {
            it->rawUpdate(data);
            // fire event.
        }
    }
}

void DebugModel::fetchCallStack(Thread &thread)
{
    // fetch whole threads.
    thread.fetchCallStack();
}

IBreakpoint convertToIBreakpoint(Breakpoint &bp)
{
    IBreakpoint ibp;
    ibp.condition = bp.condition;
    ibp.hitCondition = bp.hitCondition;
    ibp.logMessage = bp.logMessage;
    ibp.verified = bp.verified();
    ibp.support = bp.supported();
    ibp.message = bp.message();
    ibp.sessionsThatVerified = bp.sessionsThatVerified();

    ibp.uri = bp.uri();
    ibp.lineNumber = bp.lineNumber();
    ibp.endLineNumber = bp.endLineNumber();
    ibp.column = bp.column();
    ibp.endColumn = bp.endColumn();
    ibp.adapterData = bp.adapterData();

    return ibp;
}

dap::array<IBreakpoint> DebugModel::getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber, dap::optional<int> column, dap::optional<bool> enabledOnly)
{
    dap::array<IBreakpoint> ret;
    dap::string uriStr = url ? url->toString().toStdString() : "";
    for (auto it : breakPoints) {
        if ((url && it.uri().toString().toStdString() != uriStr)
            || (lineNumber && lineNumber.value() != it.lineNumber())
            || (enabledOnly && (!breakpointsActivated || enabledOnly.value() != it.enabled))) {
            continue;
        }
        auto ibp = convertToIBreakpoint(it);
        ret.push_back(ibp);
    }
    return ret;
}

QMap<QString, dap::array<IBreakpoint>> DebugModel::getAllBreakpoints()
{
    QMap<QString, dap::array<IBreakpoint>> allBreakpoints;
    for (auto it : breakPoints) {
        auto ibp = convertToIBreakpoint(it);
        QString path = ibp.uri.path();
        if (allBreakpoints.contains(path)) {
            allBreakpoints[path].push_back(ibp);
        } else {
            allBreakpoints.insert(path, {ibp});
        }
    }
    return allBreakpoints;
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

dap::array<IBreakpoint> DebugModel::addBreakpoints(
        QUrl &uri, dap::array<IBreakpointData> &rawData, bool fireEvent)
{
    Q_UNUSED(uri)
    Q_UNUSED(fireEvent)

    dap::array<IBreakpoint> retBreakpoints;
    for (auto rawBp : rawData) {
        Breakpoint bp(uri, rawBp.lineNumber.value(), rawBp.column, rawBp.enabled,
                      rawBp.condition, rawBp.hitCondition, rawBp.logMessage, undefined, rawBp.id.value());
        auto ibp = convertToIBreakpoint(bp);
        retBreakpoints.push_back(ibp);
        breakPoints.push_back(bp);
    }

    return retBreakpoints;
}

dap::array<IBreakpoint> DebugModel::removeBreakpoint(const QString &filePath, int lineNumber)
{
    for (auto bp = breakPoints.begin(); bp != breakPoints.end(); ) {
        if (bp->lineNumber() == lineNumber
                && bp->uri().toString() == filePath) {
            bp = breakPoints.erase(bp);
        } else {
            ++bp;
        }
    }

    dap::array<IBreakpoint> retBreakpoints;
    for (auto bp : breakPoints) {
        auto ibp = convertToIBreakpoint(bp);
        retBreakpoints.push_back(ibp);
    }
    return retBreakpoints;
}

dap::array<IBreakpoint> DebugModel::switchBreakpointStatus(const QString &filePath, int lineNumber, bool status)
{
    for (auto bp = breakPoints.begin(); bp != breakPoints.end(); ) {
        if (bp->lineNumber() == lineNumber
                && bp->uri().toString() == filePath) {
            bp->enabled = status;
            break;
        } else {
            ++bp;
        }
    }

    dap::array<IBreakpoint> retBreakpoints;
    for (auto bp : breakPoints) {
        auto ibp = convertToIBreakpoint(bp);
        retBreakpoints.push_back(ibp);
    }
    return retBreakpoints;
    // fire event.
}

dap::array<IBreakpoint> DebugModel::setBreakpointCondition(const QString &filePath, int lineNumber, const QString &expression)
{
    for (auto bp = breakPoints.begin(); bp != breakPoints.end(); ) {
        if (bp->lineNumber() == lineNumber
                && bp->uri().toString() == filePath) {
            bp->condition = expression.toStdString();
            break;
        } else {
            ++bp;
        }
    }

    dap::array<IBreakpoint> retBreakpoints;
    for (auto bp : breakPoints) {
        auto ibp = convertToIBreakpoint(bp);
        retBreakpoints.push_back(ibp);
    }
    return retBreakpoints;
    // fire event.
}

void DebugModel::updateBreakpoints(std::map<dap::string, IBreakpointUpdateData> &data)
{
    Q_UNUSED(data)
    //    dap::array<IBreakpoint> updated;
    //    for (auto bp : breakPoints) {
    //        auto bpData = data.find(bp.Enablement::getId());
    //        if (bpData != data.end()) {
    //            bp.update(bpData->second);
    //            updated.push_back(bp);
    //        }
    //    }
}

IBreakpointSessionData toBreakpointSessionData(dap::Breakpoint &data, dap::Capabilities &capabilities)
{
    Q_UNUSED(data)
    Q_UNUSED(capabilities)
    return {};
}

void DebugModel::setBreakpointSessionData(dap::string &sessionId, const dap::Capabilities &capabilites, dap::optional<std::map<dap::string, dap::Breakpoint>> data)
{
    for (auto bp : breakPoints) {
        if (!data) {
            bp.setSessionData(sessionId, undefined);
        } else {
            auto bpData = data.value().find(bp.getId());
            if (bpData != data->end()) {
                // TODO(mozart):write session data.
                bp.setSessionData(sessionId, undefined);
            }
        }
    }
}

dap::optional<dap::Breakpoint> DebugModel::getDebugProtocolBreakpoint(
        dap::string &breakpointId, dap::string &sessionId)
{
    for (auto bp : breakPoints) {
        if (bp.getId() == breakpointId) {
            return bp.getDebugProtocolBreakpoint(sessionId);
        }
    }
    return undefined;
}

void DebugModel::enableOrDisableAllBreakpoints(bool enable)
{
    for (auto bp : breakPoints) {
        bp.enabled = enable;
    }
}

void DebugModel::setBreakpointsActivated(bool activated)
{
    breakpointsActivated = activated;
    // fire event.
}
} // end namespace.

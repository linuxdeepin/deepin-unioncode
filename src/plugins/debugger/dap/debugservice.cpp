// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugservice.h"
#include "debugmodel.h"
#include "debugsession.h"

namespace DEBUG_NAMESPACE {


DebugService::DebugService(QObject *parent)
    : QObject(parent)
{
    model.reset(new DebugModel(undefined, this));
}

void DebugService::sendAllBreakpoints(DebugSession *session)
{
#if 0 // breakpoint type not supported now.
    sendBreakpoints(undefined, session, false);
    sendFunctionBreakpoints(session);
    sendDataBreakpoints(session);
    sendInstructionBreakpoints(session);
    // send exception breakpoints at the end since some debug adapters rely on the order
    sendExceptionBreakpoints(session);
#endif

    // remove all previous breakpoints.
    sendBreakpoints({}, session);

    auto breakpoints = model->getAllBreakpoints();
    for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
        sendBreakpoints(it.key(), session);
    }
}

dap::array<IBreakpoint> DebugService::addBreakpoints(
        QUrl uri, dap::array<IBreakpointData> rawBreakpoints, dap::optional<DebugSession *> session)
{
    auto breakpoints = model->addBreakpoints(uri, rawBreakpoints);
    if (session)
        sendBreakpoints(uri, session.value());

    return breakpoints;
}

dap::array<IBreakpoint> DebugService::removeBreakpoints(
        const QString &filePath, int lineNumber, dap::optional<DebugSession *> session)
{
    auto breakpoints = model->removeBreakpoint(filePath, lineNumber);
    if (session)
        sendBreakpoints(QUrl(filePath), session.value());

    return breakpoints;
}

void DebugService::switchBreakpointStatus(const QList<QPair<QString, int>> &breakpoints, bool status,
                                                           dap::optional<DebugSession *> session)
{
    QStringList urlList;
    for (auto bp : breakpoints) {
        if (!urlList.contains(bp.first))
            urlList.append(bp.first);
        model->switchBreakpointStatus(bp.first, bp.second, status);
    }

    if (session) {
        for (auto url : urlList)
            sendBreakpoints(QUrl(url), session.value());
    }
}

DebugModel *DebugService::getModel() const
{
    return model.get();
}

void DebugService::sendBreakpoints(dap::optional<QUrl> uri, DebugSession *session, bool sourceModified)
{
    Q_UNUSED(sourceModified)
    if (!uri.has_value()) {
        dap::array<IBreakpoint> empty;
        session->sendBreakpoints({}, empty);
    } else {
        auto breakpointsToSend = model->getBreakpoints(uri, undefined, undefined, true);
        session->sendBreakpoints(uri->path(), breakpointsToSend);
    }
}

void DebugService::sendFunctionBreakpoints(DebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendDataBreakpoints(DebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendInstructionBreakpoints(DebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendExceptionBreakpoints(DebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}
} // end namespace.

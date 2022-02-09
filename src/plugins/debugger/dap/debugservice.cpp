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
#include "debugservice.h"
#include "debugmodel.h"
#include "debugsession.h"

namespace DEBUG_NAMESPACE {


DebugService::DebugService(QObject *parent)
    : QObject(parent)
{
    model.reset(new DebugModel(undefined, this));
}

void DebugService::sendAllBreakpoints(IDebugSession *session)
{
    sendBreakpoints(undefined, session, false);
    sendFunctionBreakpoints(session);
    sendDataBreakpoints(session);
    sendInstructionBreakpoints(session);
    // send exception breakpoints at the end since some debug adapters rely on the order
    sendExceptionBreakpoints(session);
}

dap::array<IBreakpoint> DebugService::addBreakpoints(
        QUrl uri, dap::array<IBreakpointData> rawBreakpoints, dap::optional<IDebugSession *> session)
{
    auto breakpoints = model->addBreakpoints(uri, rawBreakpoints);
    if (session)
        sendBreakpoints(uri, session.value());

    return breakpoints;
}

dap::array<IBreakpoint> DebugService::removeBreakpoints(
        const QString &filePath, int lineNumber, dap::optional<IDebugSession *> session)
{
    auto breakpoints = model->removeBreakpoint(filePath, lineNumber);
    if (session)
        sendBreakpoints(QUrl(filePath), session.value());

    return breakpoints;
}

DebugModel *DebugService::getModel() const
{
    return model.get();
}

void DebugService::sendBreakpoints(dap::optional<QUrl> uri, IDebugSession *session, bool sourceModified)
{
    Q_UNUSED(sourceModified)
    auto breakpointsToSend = model->getBreakpoints(uri, undefined, undefined, true);
    session->sendBreakpoints(breakpointsToSend);
}

void DebugService::sendFunctionBreakpoints(IDebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendDataBreakpoints(IDebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendInstructionBreakpoints(IDebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}

void DebugService::sendExceptionBreakpoints(IDebugSession *session)
{
    Q_UNUSED(session)
    // TODO(mozart)
}
} // end namespace.

// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGSERVICE_H
#define DEBUGSERVICE_H

#include "debug.h"
#include "debugsession.h"

#include <QObject>
#include <QSharedPointer>

namespace DEBUG_NAMESPACE {

class DebugModel;
class DebugService : public QObject
{
    Q_OBJECT
public:
    explicit DebugService(QObject *parent = nullptr);

    void sendAllBreakpoints(DebugSession *session);
    dap::array<IBreakpoint> addBreakpoints(QUrl uri, dap::array<IBreakpointData> rawBreakpoints,
                                           dap::optional<DebugSession *> session);

    dap::array<IBreakpoint> removeBreakpoints(const QString &filePath, int lineNumber,
                                           dap::optional<DebugSession *> session);

    void switchBreakpointStatus(const QString &filePath, int lineNumber, bool status,
                                           dap::optional<DebugSession *> session);

    void setBreakpointCondition(const QString &filePath, int lineNumber, const QString &expression,
                                           dap::optional<DebugSession *> session);

    DebugModel *getModel() const;

signals:

public slots:

private:
    void sendBreakpoints(dap::optional<QUrl> uri, DebugSession *session, bool sourceModified = false);
    void sendFunctionBreakpoints(DebugSession *session);
    void sendDataBreakpoints(DebugSession *session);
    void sendInstructionBreakpoints(DebugSession *session);
    void sendExceptionBreakpoints(DebugSession *session);

    QSharedPointer<DebugModel> model;
};

} // end namespace.

#endif   // DEBUGSERVICE_H

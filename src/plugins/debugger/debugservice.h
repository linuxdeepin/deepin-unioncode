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
#ifndef DEBUGSERVICE_H
#define DEBUGSERVICE_H

#include "debug.h"
#include "debugsession.h"

#include <QObject>
#include <QSharedPointer>

class DebugModel;
class DebugService : public QObject
{
    Q_OBJECT
public:
    explicit DebugService(QObject *parent = nullptr);

    void sendAllBreakpoints(DebugSession *session);
    dap::array<IBreakpoint> addBreakpoints(QUrl uri, dap::array<IBreakpointData> rawBreakpoints,
                                           dap::optional<DebugSession *> session);

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

#endif   // DEBUGSERVICE_H

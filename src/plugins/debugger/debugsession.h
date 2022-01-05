/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef DEBUGSESSION_H
#define DEBUGSESSION_H

#include "session.h"
#include "rawdebugsession.h"
#include "protocol.h"

#include <QObject>
#include <QSharedPointer>

#include <memory>

namespace dap {
class RawDebugSession;
}

class DebugSession : public QObject
{
    Q_OBJECT
public:
    explicit DebugSession(QObject *parent = nullptr);

    bool initialize();

    void launch(bool noDebug = false);
    void attach();

    void restart();
    void terminate(bool restart = false);
    void disconnect(bool terminateDebuggee = true, bool restart = false);

    void continueDbg(int threadId);
    void pause(int threadId);

    void stepIn(int threadId, int targetId, dap::SteppingGranularity granularity);
    void stepOut(int threadId, dap::SteppingGranularity granularity);
    void next(int threadId, dap::SteppingGranularity granularity);

signals:

public slots:

private:
    void shutdown();
    QSharedPointer<dap::RawDebugSession> raw;

    bool initialized = false;

    std::shared_ptr<dap::Session> session;
};

#endif // DEBUGSESSION_H

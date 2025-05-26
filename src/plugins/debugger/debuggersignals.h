// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGERSIGNALS_H
#define DEBUGGERSIGNALS_H

#include "debuggerglobals.h"
#include "stackframe.h"
#include "interface/variable.h"
#include "event/event.h"

#include <QObject>

class DebuggerSignals : public QObject
{
    Q_OBJECT

public:
Q_SIGNALS:
    void receivedEvent(const dpf::Event &event);
};

#endif   // DEBUGGERSIGNALS_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGERGLOBALS_H
#define DEBUGGERGLOBALS_H

#include <common/util/singleton.h>
#include <QColor>
#include <QtConcurrent>

/*
 * Redefine global instance
 */
#define debuggerSignals Singleton<DebuggerSignals>::instance()
#define eventSender Singleton<EventSender>::instance()
#define eventReceiver Singleton<EventReceiver>::instance()
#define debugManager Singleton<DebugManager>::instance()
#define debugService Singleton<DebugService>::instance()

// Do some test when valued 1
#define DBG_TEST 1
#define DEBUG_NAMESPACE DEBUG

/**
 * @brief Output text color.
 */
const QColor kTextColorNormal(150, 150, 150);
const QColor kErrorMessageTextColor(255, 108, 108);
const QColor kMessageOutput(0, 135, 135);

#define AsynInvoke(Fun)          \
    QtConcurrent::run([this]() { \
        Fun;                     \
    });

#endif   // DEBUGGERGLOBALS_H

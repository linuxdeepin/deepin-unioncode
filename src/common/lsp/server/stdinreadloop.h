// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STDINREADLOOP_H
#define STDINREADLOOP_H

#include "common/common_global.h"

#include <QThread>

class StdinReadLoopPrivate;
class COMMON_EXPORT StdinReadLoop : public QThread
{
    Q_OBJECT
    StdinReadLoopPrivate *const d;
public:
    StdinReadLoop();
    virtual ~StdinReadLoop();
    virtual void run();
Q_SIGNALS:
    void readedLine(const QByteArray &);
};

#endif // STDINREADLOOP_H

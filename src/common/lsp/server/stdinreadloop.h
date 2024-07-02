// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STDINREADLOOP_H
#define STDINREADLOOP_H

#include <QThread>

class StdinReadLoopPrivate;
class StdinReadLoop : public QThread
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

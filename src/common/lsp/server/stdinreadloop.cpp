// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "stdinreadloop.h"

#include <QTextStream>

class StdinReadLoopPrivate
{
    friend class StdinReadLoop;
    QTextStream *stream{nullptr};
    StdinReadLoopPrivate() = default;
};

StdinReadLoop::StdinReadLoop()
    : QThread()
    , d (new StdinReadLoopPrivate)
{
    d->stream = new QTextStream(stdin);
}

StdinReadLoop::~StdinReadLoop()
{
    if (d) {
        if (d->stream) {
            delete d->stream;
        }
        delete d;
    }
}

void StdinReadLoop::run()
{
    QString line;
    while (d->stream->readLineInto(&line)) {
        Q_EMIT readedLine(line.toLatin1());
    }
}

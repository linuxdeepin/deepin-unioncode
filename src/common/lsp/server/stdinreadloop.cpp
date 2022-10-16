/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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

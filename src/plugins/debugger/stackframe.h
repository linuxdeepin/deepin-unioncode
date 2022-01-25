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
#ifndef STACKFRAME_H
#define STACKFRAME_H

#include <QCoreApplication>
#include <QMetaType>

class StackFrameData
{
public:
    StackFrameData();
    void clear();
    bool isUsable() const;
    QString toToolTip() const;
    QString toString() const;

public:
    QString level;
    QString function;
    QString file;        // We try to put an absolute file name in there.
    QString module;      // Sometimes something like "/usr/lib/libstdc++.so.6"
    QString receiver;    // Used in ScriptEngine only.
    qint32 line = -1;
    QString address;
    int64_t frameId = 0;
    bool usable = true;

    Q_DECLARE_TR_FUNCTIONS(StackHandler)
};

using StackFrames = QList<StackFrameData>;

Q_DECLARE_METATYPE(StackFrameData)
#endif // STACKFRAME_H

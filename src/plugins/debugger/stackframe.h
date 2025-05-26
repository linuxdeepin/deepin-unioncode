// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

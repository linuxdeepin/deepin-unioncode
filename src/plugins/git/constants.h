// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QDateTime>

struct CommitInfo
{
    QString sha1;
    QString author;
    QString authorMail;
    QDateTime authorTime;
    QString summary;
    QString filePath;
};

constexpr char GitBinaryPath[] { "/usr/bin/git" };

#endif   // CONSTANTS_H

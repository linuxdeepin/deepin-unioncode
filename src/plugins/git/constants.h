// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
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

enum FormatCodes {
    ResetFormat = 0,
    BoldText = 1,
    TextColorStart = 30,
    TextColorEnd = 37,
    RgbTextColor = 38,
    DefaultTextColor = 39,
    BackgroundColorStart = 40,
    BackgroundColorEnd = 47,
    RgbBackgroundColor = 48,
    DefaultBackgroundColor = 49
};

enum GitType {
    GitLog,
    GitBlame,
    GitDiff,
    GitShow
};

enum DiffSide {
    LeftSide,
    RightSide,
    SideCount
};

constexpr char GitWindow[] { "GitWindow" };
constexpr char GitFilePath[] { "FilePath" };
constexpr char GitIsProject[] { "IsProject" };

constexpr char GitBinaryPath[] { "/usr/bin/git" };
constexpr char GitDirectory[] { ".git" };
constexpr char DecorateOption[] { "--decorate" };
constexpr char ColorOption[] { "--color=always" };
constexpr int LogMaxCount { 100 };
constexpr char GitShowFormat[] { "--pretty=format:commit %H%d%n"
                                 "Author: %an <%ae>, %ad (%ar)%n"
                                 "Committer: %cn <%ce>, %cd (%cr)%n"
                                 "%n"
                                 "%B" };

constexpr char ChangePattern[] { "\\b[a-f0-9]{7,40}\\b" };
constexpr char AnnotationEntryPattern[] { "^(\\b[a-f0-9]{7,40}\\b) " };

#endif   // CONSTANTS_H

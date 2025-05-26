// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEFORMATHANDLER_H
#define CODEFORMATHANDLER_H

#include <QTextCharFormat>

class FormattedText
{
public:
    FormattedText() = default;
    explicit FormattedText(const QString &txt, const QTextCharFormat &fmt = QTextCharFormat())
        : text(txt), format(fmt) {}

    QString text;
    QTextCharFormat format;
};

class CodeFormatHandler
{
public:
    QList<FormattedText> parseText(const FormattedText &input);
    void endFormatScope();

private:
    void setFormatScope(const QTextCharFormat &charFormat);

    bool previousFormatClosed = true;
    bool waitingForTerminator = false;
    QString alternateTerminator;
    QTextCharFormat previousFormat;
    QString pendingText;
};

#endif   // CODEFORMATHANDLER_H

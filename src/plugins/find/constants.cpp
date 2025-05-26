// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "constants.h"

QString Utils::expandRegExpReplacement(const QString &replaceText, const QStringList &capturedTexts)
{
    // handles \1 \\ \& \t \n $1 $$ $&
    QString result;
    const int numCaptures = capturedTexts.size() - 1;
    const int replaceLength = replaceText.length();
    for (int i = 0; i < replaceLength; ++i) {
        QChar c = replaceText.at(i);
        if (c == QLatin1Char('\\') && i < replaceLength - 1) {
            c = replaceText.at(++i);
            if (c == QLatin1Char('\\')) {
                result += QLatin1Char('\\');
            } else if (c == QLatin1Char('&')) {
                result += QLatin1Char('&');
            } else if (c == QLatin1Char('t')) {
                result += QLatin1Char('\t');
            } else if (c == QLatin1Char('n')) {
                result += QLatin1Char('\n');
            } else if (c.isDigit()) {
                int index = c.unicode() - '1';
                if (index < numCaptures) {
                    result += capturedTexts.at(index + 1);
                }   // else add nothing
            } else {
                result += QLatin1Char('\\');
                result += c;
            }
        } else if (c == '$' && i < replaceLength - 1) {
            c = replaceText.at(++i);
            if (c == '$') {
                result += '$';
            } else if (c == '&') {
                result += capturedTexts.at(0);
            } else if (c.isDigit()) {
                int index = c.unicode() - '1';
                if (index < numCaptures) {
                    result += capturedTexts.at(index + 1);
                }   // else add nothing
            } else {
                result += '$';
                result += c;
            }
        } else {
            result += c;
        }
    }
    return result;
}

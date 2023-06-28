// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ansifilterparser.h"

namespace {
enum AnsiState {
    PLAIN,
    ANSI_START,
    ANSI_CSI,
    ANSI_SEQUENCE,
    ANSI_WAITING_FOR_ST,
    ANSI_ST_STARTED
};
} // namespace

AnsiFilterParser::AnsiFilterParser()
{
    setObjectName(QLatin1String("AnsiFilterParser"));
}

void AnsiFilterParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    IOutputParser::stdOutput(filterLine(line), format);
}

void AnsiFilterParser::stdError(const QString &line)
{
    IOutputParser::stdError(filterLine(line));
}

QString AnsiFilterParser::filterLine(const QString &line)
{
    QString result;
    result.reserve(line.count());

    static AnsiState state = PLAIN;
    foreach (const QChar c, line) {
        unsigned int val = c.unicode();
        switch (state) {
        case PLAIN:
            if (val == 27) // 'ESC'
                state = ANSI_START;
            else if (val == 155) // equivalent to 'ESC'-'['
                state = ANSI_CSI;
            else
                result.append(c);
            break;
        case ANSI_START:
            if (val == 91) // [
                state = ANSI_CSI;
            else if (val == 80 || val == 93 || val == 94 || val == 95) // 'P', ']', '^' and '_'
                state = ANSI_WAITING_FOR_ST;
            else if (val >= 64 && val <= 95)
                state = PLAIN;
            else
                state = ANSI_SEQUENCE;
            break;
        case ANSI_CSI:
            if (val >= 64 && val <= 126) // Anything between '@' and '~'
                state = PLAIN;
            break;
        case ANSI_SEQUENCE:
            if (val >= 64 && val <= 95) // Anything between '@' and '_'
                state = PLAIN;
            break;
        case ANSI_WAITING_FOR_ST:
            if (val == 7) // 'BEL'
                state = PLAIN;
            if (val == 27) // 'ESC'
                state = ANSI_ST_STARTED;
            break;
        case ANSI_ST_STARTED:
            if (val == 92) // '\'
                state = PLAIN;
            else
                state = ANSI_WAITING_FOR_ST;
            break;
        }
    }
    return result;
}

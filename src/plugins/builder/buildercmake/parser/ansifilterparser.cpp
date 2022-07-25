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

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeformathandler.h"
#include "constants.h"

#include "common/util/qtcassert.h"

static QColor textColor(uint code)
{
    QTC_ASSERT(code < 8, return QColor());

    const int red = code & 1 ? 170 : 0;
    const int green = code & 2 ? 170 : 0;
    const int blue = code & 4 ? 170 : 0;
    return QColor(red, green, blue);
}

QList<FormattedText> CodeFormatHandler::parseText(const FormattedText &input)
{
    const QString escape = "\x1b[";
    const QChar semicolon = ';';
    const QChar colorTerminator = 'm';
    const QChar eraseToEol = 'K';

    QList<FormattedText> outputData;
    QTextCharFormat charFormat = previousFormatClosed ? input.format : previousFormat;
    QString strippedText;
    if (pendingText.isEmpty()) {
        strippedText = input.text;
    } else {
        strippedText = pendingText.append(input.text);
        pendingText.clear();
    }

    while (!strippedText.isEmpty()) {
        QTC_ASSERT(pendingText.isEmpty(), break);
        if (waitingForTerminator) {
            // We ignore all escape codes taking string arguments.
            QString terminator = "\x1b\\";
            int terminatorPos = strippedText.indexOf(terminator);
            if (terminatorPos == -1 && !alternateTerminator.isEmpty()) {
                terminator = alternateTerminator;
                terminatorPos = strippedText.indexOf(terminator);
            }
            if (terminatorPos == -1) {
                pendingText = strippedText;
                break;
            }
            waitingForTerminator = false;
            alternateTerminator.clear();
            strippedText.remove(0, terminatorPos + terminator.length());
            if (strippedText.isEmpty())
                break;
        }
        const int escapePos = strippedText.indexOf(escape.at(0));
        if (escapePos < 0) {
            outputData << FormattedText(strippedText, charFormat);
            break;
        } else if (escapePos != 0) {
            outputData << FormattedText(strippedText.left(escapePos), charFormat);
            strippedText.remove(0, escapePos);
        }
        QTC_ASSERT(strippedText.at(0) == escape.at(0), break);

        while (!strippedText.isEmpty() && escape.at(0) == strippedText.at(0)) {
            if (escape.startsWith(strippedText)) {
                // control secquence is not complete
                pendingText += strippedText;
                strippedText.clear();
                break;
            }
            if (!strippedText.startsWith(escape)) {
                switch (strippedText.at(1).toLatin1()) {
                case '\\':   // Unexpected terminator sequence.
                    QTC_CHECK(false);
                    Q_FALLTHROUGH();
                case 'N':
                case 'O':   // Ignore unsupported single-character sequences.
                    strippedText.remove(0, 2);
                    break;
                case ']':
                    alternateTerminator = QChar(7);
                    Q_FALLTHROUGH();
                case 'P':
                case 'X':
                case '^':
                case '_':
                    strippedText.remove(0, 2);
                    waitingForTerminator = true;
                    break;
                default:
                    // not a control sequence
                    pendingText.clear();
                    outputData << FormattedText(strippedText.left(1), charFormat);
                    strippedText.remove(0, 1);
                    continue;
                }
                break;
            }
            pendingText += strippedText.mid(0, escape.length());
            strippedText.remove(0, escape.length());

            // \e[K is not supported. Just strip it.
            if (strippedText.startsWith(eraseToEol)) {
                pendingText.clear();
                strippedText.remove(0, 1);
                continue;
            }
            // get the number
            QString strNumber;
            QStringList numbers;
            while (!strippedText.isEmpty()) {
                if (strippedText.at(0).isDigit()) {
                    strNumber += strippedText.at(0);
                } else {
                    if (!strNumber.isEmpty())
                        numbers << strNumber;
                    if (strNumber.isEmpty() || strippedText.at(0) != semicolon)
                        break;
                    strNumber.clear();
                }
                pendingText += strippedText.mid(0, 1);
                strippedText.remove(0, 1);
            }
            if (strippedText.isEmpty())
                break;

            // remove terminating char
            if (!strippedText.startsWith(colorTerminator)) {
                pendingText.clear();
                strippedText.remove(0, 1);
                break;
            }
            // got consistent control sequence, ok to clear pending text
            pendingText.clear();
            strippedText.remove(0, 1);

            if (numbers.isEmpty()) {
                charFormat = input.format;
                endFormatScope();
            }

            for (int i = 0; i < numbers.size(); ++i) {
                const uint code = numbers.at(i).toUInt();

                if (code >= TextColorStart && code <= TextColorEnd) {
                    charFormat.setForeground(textColor(code - TextColorStart));
                    setFormatScope(charFormat);
                } else if (code >= BackgroundColorStart && code <= BackgroundColorEnd) {
                    charFormat.setBackground(textColor(code - BackgroundColorStart));
                    setFormatScope(charFormat);
                } else {
                    switch (code) {
                    case ResetFormat:
                        charFormat = input.format;
                        endFormatScope();
                        break;
                    case BoldText:
                        charFormat.setFontWeight(QFont::Bold);
                        setFormatScope(charFormat);
                        break;
                    case DefaultTextColor:
                        charFormat.setForeground(input.format.foreground());
                        setFormatScope(charFormat);
                        break;
                    case DefaultBackgroundColor:
                        charFormat.setBackground(input.format.background());
                        setFormatScope(charFormat);
                        break;
                    case RgbTextColor:
                    case RgbBackgroundColor:
                        // See http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
                        if (++i >= numbers.size())
                            break;
                        switch (numbers.at(i).toInt()) {
                        case 2:
                            // RGB set with format: 38;2;<r>;<g>;<b>
                            if ((i + 3) < numbers.size()) {
                                (code == RgbTextColor) ? charFormat.setForeground(QColor(numbers.at(i + 1).toInt(),
                                                                                         numbers.at(i + 2).toInt(),
                                                                                         numbers.at(i + 3).toInt()))
                                                       : charFormat.setBackground(QColor(numbers.at(i + 1).toInt(),
                                                                                         numbers.at(i + 2).toInt(),
                                                                                         numbers.at(i + 3).toInt()));
                                setFormatScope(charFormat);
                            }
                            i += 3;
                            break;
                        case 5:
                            // 256 color mode with format: 38;5;<i>
                            uint index = numbers.at(i + 1).toUInt();

                            QColor color;
                            if (index < 8) {
                                // The first 8 colors are standard low-intensity ANSI colors.
                                color = textColor(index);
                            } else if (index < 16) {
                                // The next 8 colors are standard high-intensity ANSI colors.
                                color = textColor(index - 8).lighter(150);
                            } else if (index < 232) {
                                // The next 216 colors are a 6x6x6 RGB cube.
                                uint o = index - 16;
                                color = QColor((o / 36) * 51, ((o / 6) % 6) * 51, (o % 6) * 51);
                            } else {
                                // The last 24 colors are a greyscale gradient.
                                int grey = int((index - 232) * 11);
                                color = QColor(grey, grey, grey);
                            }

                            if (code == RgbTextColor)
                                charFormat.setForeground(color);
                            else
                                charFormat.setBackground(color);

                            setFormatScope(charFormat);
                            ++i;
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    return outputData;
}

void CodeFormatHandler::endFormatScope()
{
    previousFormatClosed = true;
}

void CodeFormatHandler::setFormatScope(const QTextCharFormat &charFormat)
{
    previousFormat = charFormat;
    previousFormatClosed = false;
}

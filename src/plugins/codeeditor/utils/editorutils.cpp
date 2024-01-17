// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorutils.h"
#include "defaultlexer.h"
#include "common/common.h"

QsciLexer *EditorUtils::defaultLexer(const QString &fileName)
{
    using namespace support_file;
    auto id = Language::id(fileName);

    QsciLexer *lexer { nullptr };
    if (id.compare("cpp", Qt::CaseInsensitive)) {
        lexer = new QsciLexerCPP();
    } else if (id.compare("java", Qt::CaseInsensitive)) {
        lexer = new QsciLexerJava();
    } else if (id.compare("cmake", Qt::CaseInsensitive)) {
        lexer = new QsciLexerCMake();
    } else if (id.compare("json", Qt::CaseInsensitive)) {
        lexer = new QsciLexerJSON();
    } else if (id.compare("xml", Qt::CaseInsensitive)) {
        lexer = new QsciLexerXML();
    } else if (id.compare("python", Qt::CaseInsensitive)) {
        lexer = new QsciLexerPython();
    } else if (id.compare("js", Qt::CaseInsensitive)) {
        lexer = new QsciLexerJavaScript();
    }

    return lexer;
}

int EditorUtils::nbDigitsFromNbLines(long nbLines)
{
    int nbDigits = 0;   // minimum number of digit should be 4
    if (nbLines < 10)
        nbDigits = 1;
    else if (nbLines < 100)
        nbDigits = 2;
    else if (nbLines < 1000)
        nbDigits = 3;
    else if (nbLines < 10000)
        nbDigits = 4;
    else if (nbLines < 100000)
        nbDigits = 5;
    else if (nbLines < 1000000)
        nbDigits = 6;
    else {   // rare case
        nbDigits = 7;
        nbLines /= 1000000;

        while (nbLines) {
            nbLines /= 10;
            ++nbDigits;
        }
    }

    return nbDigits;
}

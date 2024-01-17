// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORUTILS_H
#define EDITORUTILS_H

#include <Qsci/qscilexer.h>

class EditorUtils
{
public:
    static QsciLexer *defaultLexer(const QString &fileName);
    static int nbDigitsFromNbLines(long nbLines);
};

#endif   // EDITORUTILS_H

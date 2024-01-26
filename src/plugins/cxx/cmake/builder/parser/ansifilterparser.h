// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANSIFILTERPARSER_H
#define ANSIFILTERPARSER_H

#include "base/abstractoutputparser.h"

class AnsiFilterParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    AnsiFilterParser();
    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;

private:
    QString filterLine(const QString &line);
};

#endif // ANSIFILTERPARSER_H

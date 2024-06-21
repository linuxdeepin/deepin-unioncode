// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLPARSER_H
#define LLPARSER_H

#include "base/abstractoutputparser.h"

class LLParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    explicit LLParser();

    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
    void taskAdded(const Task &task, int linkedLines, int skippedLines) override;

private:
};

#endif   // NINJAPARSER_H

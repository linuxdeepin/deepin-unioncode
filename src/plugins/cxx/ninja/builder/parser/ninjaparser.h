// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAPARSER_H
#define NINJAPARSER_H

#include "base/abstractoutputparser.h"

class NinjaParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    explicit NinjaParser();

    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
    void taskAdded(const Task &task, int linkedLines, int skippedLines) override;

private:

};

#endif // NINJAPARSER_H

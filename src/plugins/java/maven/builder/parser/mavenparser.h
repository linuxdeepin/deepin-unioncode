// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENPARSER_H
#define MAVENPARSER_H

#include "base/abstractoutputparser.h"

class MavenParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    explicit MavenParser();

    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
    void taskAdded(const Task &task, int linkedLines, int skippedLines) override;

private:
};

#endif // MAVENPARSER_H

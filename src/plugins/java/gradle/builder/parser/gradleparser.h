// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEPARSER_H
#define GRADLEPARSER_H

#include "base/abstractoutputparser.h"

class GradleParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    explicit GradleParser();

    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
    void taskAdded(const Task &task, int linkedLines, int skippedLines) override;

private:

};

#endif // GRADLEPARSER_H

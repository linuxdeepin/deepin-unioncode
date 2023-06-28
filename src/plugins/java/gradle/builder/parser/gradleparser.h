// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEPARSER_H
#define GRADLEPARSER_H

#include "services/builder/ioutputparser.h"

class GradleParser : public IOutputParser
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

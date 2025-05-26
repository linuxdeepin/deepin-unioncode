// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GNUMAKEPARSER_H
#define GNUMAKEPARSER_H

#include "base/abstractoutputparser.h"

#include <QRegularExpression>
#include <QStringList>

class GnuMakeParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    explicit GnuMakeParser();

    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;

    void setWorkingDirectory(const QString &workingDirectory) override;

    QStringList searchDirectories() const;

    bool hasFatalErrors() const override;

    void taskAdded(const Task &task, int linkedLines, int skippedLines) override;

private:
    void addDirectory(const QString &dir);
    void removeDirectory(const QString &dir);

    QRegularExpression makeDir;
    QRegularExpression makeLine;
    QRegularExpression threeStarError;
    QRegularExpression errorInMakefile;

    QStringList directories;

    bool suppressIssues = false;

    int fatalErrorCount = 0;
};

#endif // GNUMAKEPARSER_H

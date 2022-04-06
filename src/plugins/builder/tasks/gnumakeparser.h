/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GNUMAKEPARSER_H
#define GNUMAKEPARSER_H

#include "ioutputparser.h"

#include <QRegularExpression>
#include <QStringList>

class GnuMakeParser : public IOutputParser
{
    Q_OBJECT

public:
    explicit GnuMakeParser();

    void stdOutput(const QString &line) override;
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

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GCCPARSER_H
#define GCCPARSER_H

#include "services/builder/ioutputparser.h"
#include "services/builder/task.h"

#include <QRegularExpression>

class GccParser : public IOutputParser
{
    Q_OBJECT

public:
    GccParser();

    void stdError(const QString &line) override;
    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;

    static QString id();

protected:
    void newTask(const Task &task);
    void doFlush() override;

    void amendDescription(const QString &desc, bool monospaced);

private:
    QRegularExpression regExp;
    QRegularExpression regExpIncluded;
    QRegularExpression regExpGccNames;

    Task currentTask;
    int lines = 0;
};

#endif // GCCPARSER_H

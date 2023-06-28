// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEPARSER_H
#define CMAKEPARSER_H

#include "services/builder/ioutputparser.h"
#include "services/builder/task.h"

#include <QObject>
#include <QRegularExpression>

class CMakeParser : public IOutputParser
{
    Q_OBJECT
public:
    explicit CMakeParser();
    virtual void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;

protected:
    void doFlush() override;

private:
    enum TripleLineError { NONE, LINE_LOCATION, LINE_DESCRIPTION, LINE_DESCRIPTION2 };

    TripleLineError expectTripleLineErrorData = NONE;

    Task lastTask;
    QRegExp commonError;
    QRegExp nextSubError;
    QRegularExpression locationLine;
    bool skippedFirstEmptyLine = false;
    int lines = 0;
};

#endif // CMAKEPARSER_H

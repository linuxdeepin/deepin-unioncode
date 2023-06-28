// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONPARSER_H
#define COMMONPARSER_H

#include "services/builder/ioutputparser.h"

class CommonParser : public IOutputParser
{
    Q_OBJECT
public:
    CommonParser();
    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
};

#endif // ANSIFILTERPARSER_H

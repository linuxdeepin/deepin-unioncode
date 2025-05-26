// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONPARSER_H
#define COMMONPARSER_H

#include "base/abstractoutputparser.h"

class CommonParser : public AbstractOutputParser
{
    Q_OBJECT
public:
    CommonParser();
    void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;
};

#endif // ANSIFILTERPARSER_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonparser.h"

CommonParser::CommonParser()
{
    setObjectName(QLatin1String("CommonParser"));
}

void CommonParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    AbstractOutputParser::stdOutput(line, format);
}

void CommonParser::stdError(const QString &line)
{
    AbstractOutputParser::stdError(line);
}


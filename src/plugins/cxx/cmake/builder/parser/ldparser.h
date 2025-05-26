// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LDPARSER_H
#define LDPARSER_H

#include "base/abstractoutputparser.h"
#include <QRegularExpression>

class LdParser : public AbstractOutputParser
{
    Q_OBJECT

public:
    LdParser();
    void stdError(const QString &line) override;

private:
    QRegularExpression ranlib;
    QRegularExpression regExpLinker;
    QRegularExpression regExpGccNames;
};

#endif // LDPARSER_H

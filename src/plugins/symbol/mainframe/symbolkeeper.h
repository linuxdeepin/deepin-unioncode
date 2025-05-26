// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLKEEPER_H
#define SYMBOLKEEPER_H

#include "symboltreeview.h"
#include "symbolparser.h"

#include <QObject>

class SymbolKeeper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SymbolKeeper)
public:
    SymbolKeeper();
    static SymbolKeeper *instance();
    SymbolTreeView *treeView();

public slots:
    void doParse(const SymbolParseArgs &args);

private slots:
    void jumpToLine(const QString &filePath, const QString &fileLine);
    void doParseDone(bool result);
};

#endif // SYMBOLKEEPER_H

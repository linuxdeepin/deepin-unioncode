// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLLOCATOR_H
#define SYMBOLLOCATOR_H

#include "base/abstractlocator.h"

class WorkspaceWidget;
class SymbolLocatorPrivate;
class SymbolLocator : public abstractLocator
{
    Q_OBJECT
public:
    explicit SymbolLocator(QObject *parent = nullptr);
    ~SymbolLocator();

    void setWorkspaceWidget(WorkspaceWidget *workspace);
    void prepareSearch(const QString &searchText) override;
    QList<baseLocatorItem> matchesFor(const QString &inputText) override;
    void accept(baseLocatorItem item) override;

private:
    SymbolLocatorPrivate *const d;
};

#endif   // SYMBOLLOCATOR_H

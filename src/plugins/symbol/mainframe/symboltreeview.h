// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLTREEVIEW_H
#define SYMBOLTREEVIEW_H

#include "services/project/projectservice.h"

#include <DTreeView>

DWIDGET_USE_NAMESPACE
class DStandardItem;
class SymbolTreeViewPrivate;
class SymbolTreeView : public DTreeView
{
    Q_OBJECT
    SymbolTreeViewPrivate *const d;
public:
    explicit SymbolTreeView(DWidget *parent = nullptr);
    virtual ~SymbolTreeView();
    void setRootPath(const QString &filePath);

signals:
    void jumpToLine(const QString &filePath, const QString &line);

private slots:
    void doDoubleClieked(const QModelIndex &index);
    void doContextMenu(const QPoint &point);
};

#endif // SYMBOLTREEVIEW_H

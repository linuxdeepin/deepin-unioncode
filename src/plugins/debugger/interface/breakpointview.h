// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BREAKPOINTVIEW_H
#define BREAKPOINTVIEW_H

#include <DTreeView>

#include <QHeaderView>

class BreakpointView : public DTK_WIDGET_NAMESPACE::DTreeView
{
public:
    explicit BreakpointView(QWidget *parent = nullptr);
    ~BreakpointView();

private:
    QModelIndex eventIndex(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *event) override;
    void initHeaderView();

    void enableBreakpoints(const QModelIndexList &rows);
    void disableBreakpoints(const QModelIndexList &rows);
    void removeBreakpoints(const QModelIndexList &rows);

    QHeaderView *headerView {nullptr};
};

#endif // BREAKPOINTVIEW_H

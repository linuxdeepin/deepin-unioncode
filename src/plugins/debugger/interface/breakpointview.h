// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BREAKPOINTVIEW_H
#define BREAKPOINTVIEW_H

#include <DTreeView>

#include <QHeaderView>

namespace dpfservice {
class EditorService;
}

class BreakpointView : public DTK_WIDGET_NAMESPACE::DTreeView
{
    Q_OBJECT
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
    void editBreakpointCondition(const QModelIndex &idnex);

    QHeaderView *headerView { nullptr };
    dpfservice::EditorService *editService { nullptr };
};

#endif   // BREAKPOINTVIEW_H

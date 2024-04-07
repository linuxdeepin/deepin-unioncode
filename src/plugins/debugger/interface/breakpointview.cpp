// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "breakpointview.h"
#include "common/common.h"
#include "breakpointmodel.h"

#include <QDebug>
#include <QFontMetrics>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

BreakpointView::BreakpointView(QWidget *parent)
    : DTreeView(parent)
{
    initHeaderView();
    setHeader(headerView);
    setTextElideMode(Qt::TextElideMode::ElideLeft);
    setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFrameStyle(QFrame::NoFrame);
    setAlternatingRowColors(true);
}

BreakpointView::~BreakpointView()
{
}

void BreakpointView::contextMenuEvent(QContextMenuEvent *event)
{
    DTreeView::contextMenuEvent(event);
    BreakpointModel* bpModel = static_cast<BreakpointModel *>(model());

    //selectedRows
    auto rows = selectionModel()->selectedRows();
    using bp = QPair<QString, int>;
    QList<bp> selectedBps;
    for (auto row : rows) {
        auto bp = bpModel->BreakpointAt(row.row());
        selectedBps.append(qMakePair(bp.filePath, bp.lineNumber));
    }

    QMenu menu(this);
    menu.addAction(tr("Enable selected breakpoints"), this, [=]() {
        debugger.enableBreakpoints(selectedBps);
    });
    menu.addAction(tr("Disable selected breakpoints"), this, [=]() {
        debugger.disableBreakpoints(selectedBps);
    });

    menu.addAction(tr("Remove selected breakpoint"), this, [=](){
        for (auto breakpoint : selectedBps)
            editor.removeBreakpoint(breakpoint.first, breakpoint.second);
    });

    //all rows
    QList<bp> allBps;
    for(int index = 0; index < bpModel->breakpointSize(); index++) {
        auto bpItem = bpModel->BreakpointAt(index);
        allBps.append(qMakePair(bpItem.filePath, bpItem.lineNumber));
    }
    menu.addAction(tr("Enable all breakpoints"), this, [=](){
        debugger.enableBreakpoints(allBps);
    });
    menu.addAction(tr("Disable all breakpoints"), this, [=](){
        debugger.disableBreakpoints(allBps);
    });

    menu.exec(event->globalPos());
}

void BreakpointView::initHeaderView()
{
    headerView = new QHeaderView(Qt::Orientation::Horizontal);
    headerView->setDefaultSectionSize(68);
    headerView->setDefaultAlignment(Qt::AlignLeft);
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    headerView->setStretchLastSection(true);
}

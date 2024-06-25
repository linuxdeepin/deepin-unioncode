// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "breakpointview.h"
#include "common/common.h"
#include "breakpointmodel.h"
#include "base/baseitemdelegate.h"
#include "services/editor/editorservice.h"

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
    setItemDelegate(new BaseItemDelegate(this));
    editService = dpfGetService(dpfservice::EditorService);

    connect(this, &QAbstractItemView::clicked,
            this, [=](const QModelIndex &index) {
                BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
                bpModel->setCurrentIndex(index.row());
            });
}

BreakpointView::~BreakpointView()
{
}

void BreakpointView::contextMenuEvent(QContextMenuEvent *event)
{
    DTreeView::contextMenuEvent(event);
    BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
    if (bpModel->breakpointSize() == 0)
        return;

    bool allSelectEnabled = true;
    bool allSelectDisabled = true;
    //selectedRows
    auto rows = selectionModel()->selectedRows();
    for (auto row : rows) {
        auto bp = bpModel->BreakpointAt(row.row());
        if (bp.enabled)
            allSelectDisabled = false;
        else
            allSelectEnabled = false;
    }

    //all rows
    bool allEnabled = true;
    bool allDisabled = true;
    QModelIndexList allRows;
    for (int index = 0; index < bpModel->breakpointSize(); index++) {
        allRows.append(bpModel->index(index, 0));
        auto bp = bpModel->BreakpointAt(index);
        if (bp.enabled)
            allDisabled = false;
        else
            allEnabled = false;
    }

    QMenu menu(this);
    if (!allSelectEnabled)
        menu.addAction(tr("Enable selected breakpoints"), this, [=]() { enableBreakpoints(rows); });
    if (!allSelectDisabled)
        menu.addAction(tr("Disable selected breakpoints"), this, [=]() { disableBreakpoints(rows); });
    menu.addSeparator();

    if (!rows.isEmpty())
        menu.addAction(tr("Remove selected breakpoints"), this, [=]() { removeBreakpoints(rows); });

    menu.addAction(tr("Remove all breakpoints"), this, [=]() { removeBreakpoints(allRows); });
    menu.addSeparator();

    if (!allEnabled)
        menu.addAction(tr("Enable all breakpoints"), this, [=]() { enableBreakpoints(allRows); });
    if (!allDisabled)
        menu.addAction(tr("Disable all breakpoints"), this, [=]() { disableBreakpoints(allRows); });

    // select one item
    if (rows.size() == 1) {
        menu.addAction(tr("Edit Condition"), this, [=]() { editBreakpointCondition(rows.at(0)); });
    }

    menu.exec(event->globalPos());
}

void BreakpointView::enableBreakpoints(const QModelIndexList &rows)
{
    BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
    QStringList openedFiles = editService->openedFiles();
    for (auto row : rows) {
        auto bp = bpModel->BreakpointAt(row.row());
        if (openedFiles.contains(bp.filePath))
            editor.setBreakpointEnabled(bp.filePath, bp.lineNumber, true);
        else
            editor.breakpointStatusChanged(bp.filePath, bp.lineNumber, true);
    }
}

void BreakpointView::disableBreakpoints(const QModelIndexList &rows)
{
    BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
    QStringList openedFiles = editService->openedFiles();
    for (auto row : rows) {
        auto bp = bpModel->BreakpointAt(row.row());
        if (openedFiles.contains(bp.filePath))
            editor.setBreakpointEnabled(bp.filePath, bp.lineNumber, false);
        else
            editor.breakpointStatusChanged(bp.filePath, bp.lineNumber, false);
    }
}

void BreakpointView::removeBreakpoints(const QModelIndexList &rows)
{
    BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
    QStringList openedFiles = editService->openedFiles();
    for (auto row : rows) {
        auto bp = bpModel->BreakpointAt(row.row());
        if (openedFiles.contains(bp.filePath))
            editor.removeBreakpoint(bp.filePath, bp.lineNumber);
        else
            editor.breakpointRemoved(bp.filePath, bp.lineNumber);
    }
}

void BreakpointView::editBreakpointCondition(const QModelIndex &index)
{
    BreakpointModel *bpModel = static_cast<BreakpointModel *>(model());
    auto bp = bpModel->BreakpointAt(index.row());

    editor.setBreakpointCondition(bp.filePath, bp.lineNumber);
}

void BreakpointView::initHeaderView()
{
    headerView = new QHeaderView(Qt::Orientation::Horizontal);
    headerView->setDefaultSectionSize(68);
    headerView->setDefaultAlignment(Qt::AlignLeft);
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    headerView->setStretchLastSection(true);
}

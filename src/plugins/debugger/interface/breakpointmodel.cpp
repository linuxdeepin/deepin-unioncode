// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "breakpointmodel.h"

#include <QStyledItemDelegate>

BreakpointModel::BreakpointModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    setObjectName("BreakpointModel");
}

BreakpointModel::~BreakpointModel() = default;

void BreakpointModel::setBreakpoints(const Internal::Breakpoints &breakpoints, bool canExpand)
{
    beginResetModel();
    contentsValid = true;
    this->canExpand = canExpand;
    bps.clear();
    for (auto it : breakpoints) {
        bps.push_back(BreakpointItem(it));
    }
    if (bps.size() >= 0)
        setCurrentIndex(0);
    else
        currentIndex = -1;
    endResetModel();
    emit breakpointChanged();
}

void BreakpointModel::insertBreakpoint(const Internal::Breakpoint &breakpoint)
{
    beginResetModel();
    auto it = bps.begin();
    for (; it != bps.end(); ++it) {
        if (it->breakpoint() == breakpoint) {
            return;
        }
    }
    contentsValid = true;
    bps.push_back(BreakpointItem(breakpoint));
    endResetModel();
    emit breakpointChanged();
}

void BreakpointModel::removeBreakpoint(const Internal::Breakpoint &breakpoint)
{
    beginResetModel();
    auto it = bps.begin();
    for (; it != bps.end();) {
        if (it->breakpoint() == breakpoint) {
            it = bps.erase(it);
        } else {
            ++it;
        }
    }
    endResetModel();
    emit breakpointChanged();
}

void BreakpointModel::switchBreakpointStatus(const Internal::Breakpoint &breakpoint)
{
    beginResetModel();
    auto it = bps.begin();
    for (; it != bps.end();) {
        if (it->breakpoint() == breakpoint && it->isEnabled() != breakpoint.enabled)
            it->setEnabled(breakpoint.enabled);
        it++;
    }
    endResetModel();
    emit breakpointChanged();
}

void BreakpointModel::setBreakpointCondition(const Internal::Breakpoint &breakpoint)
{
    beginResetModel();
    auto it = bps.begin();
    for (; it != bps.end();) {
        if (it->breakpoint() == breakpoint) {
            it->setCondition(breakpoint.condition);
            break;
        }
        it++;
    }
    endResetModel();
    emit breakpointChanged();
}

void BreakpointModel::setCurrentIndex(int level)
{
    if (level == -1 || level == currentIndex)
        return;

    // Emit changed for previous frame
    QModelIndex i = index(currentIndex, 0);
    emit dataChanged(i, i);

    currentIndex = level;
    emit currentIndexChanged();

    // Emit changed for new frame
    i = index(currentIndex, 0);
    emit dataChanged(i, i);
}

void BreakpointModel::removeAll()
{
    beginResetModel();
    bps.clear();
    setCurrentIndex(-1);
    endResetModel();
}

Internal::Breakpoint BreakpointModel::currentBreakpoint() const
{
    if (currentIndex == -1)
        return Internal::Breakpoint();

    return bps.at(currentIndex).breakpoint();
}

int BreakpointModel::rowCount(const QModelIndex &parent) const
{
    // Since the stack is not a tree, row count is 0 for any valid parent
    return parent.isValid() ? 0 : (bps.size() + canExpand);
}

int BreakpointModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : kStackColumnCount;
}

QVariant BreakpointModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= bps.size() + canExpand)
        return QVariant();

    if (index.row() == bps.size()) {
        if (role == Qt::DisplayRole && index.column() == kIndexColumn)
            return tr("...");
        if (role == Qt::DisplayRole && index.column() == kFunctionNameColumn)
            return tr("<More>");
        if (role == Qt::DecorationRole && index.column() == kIndexColumn)
            return "";
        return QVariant();
    }

    const BreakpointItem &bp = bps.at(index.row());
    return bp.data(index.row(), index.column(), role);
}

QVariant BreakpointModel::headerData(int section, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case kIndexColumn:
            return tr("Index");
        case kIsEnabled:
            return tr("Status");
        case kFunctionNameColumn:
            return tr("Function");
        case kFileNameColumn:
            return tr("File");
        case kLineNumberColumn:
            return tr("Line");
        case kCondition:
            return tr("Condition");
        case kAddressColumn:
            return tr("Address");
        };
    }
    return QVariant();
}

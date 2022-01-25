/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stackframemodel.h"

enum StackColumns {
    kStackLevelColumn,
    kStackFunctionNameColumn,
    kStackFileNameColumn,
    kStackLineNumberColumn,
    kStackAddressColumn,
    kStackColumnCount
};

StackFrameModel::StackFrameModel()
{
    setObjectName("StackModel");
}

StackFrameModel::~StackFrameModel() = default;

int StackFrameModel::rowCount(const QModelIndex &parent) const
{
    // Since the stack is not a tree, row count is 0 for any valid parent
    return parent.isValid() ? 0 : (stackFrames.size() + canExpand);
}

int StackFrameModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : kStackColumnCount;
}

QVariant StackFrameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= stackFrames.size() + canExpand)
        return QVariant();

    if (index.row() == stackFrames.size()) {
        if (role == Qt::DisplayRole && index.column() == kStackLevelColumn)
            return tr("...");
        if (role == Qt::DisplayRole && index.column() == kStackFunctionNameColumn)
            return tr("<More>");
        if (role == Qt::DecorationRole && index.column() == kStackLevelColumn)
            return "";
        return QVariant();
    }

    const StackFrameData &frame = stackFrames.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case kStackLevelColumn:
            return QString::number(index.row() + 1);
        case kStackFunctionNameColumn:
            return frame.function;
        case kStackFileNameColumn:
            return frame.file.isEmpty() ? frame.module : frame.file;
        case kStackLineNumberColumn:
            return frame.line > 0 ? QVariant(frame.line) : QVariant();
        case kStackAddressColumn:
            return frame.address;
        }
        return QVariant();
    }

    if (role == Qt::ToolTipRole)
        return frame.toToolTip();

    return QVariant();
}

QVariant StackFrameModel::headerData(int section, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case kStackLevelColumn:
            return tr("Level");
        case kStackFunctionNameColumn:
            return tr("Function");
        case kStackFileNameColumn:
            return tr("File");
        case kStackLineNumberColumn:
            return tr("Line");
        case kStackAddressColumn:
            return tr("Address");
        };
    }
    return QVariant();
}

void StackFrameModel::setCurrentIndex(int level)
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

void StackFrameModel::removeAll()
{
    beginResetModel();
    stackFrames.clear();
    setCurrentIndex(-1);
    endResetModel();
}

StackFrameData StackFrameModel::currentFrame() const
{
    if (currentIndex == -1)
        return StackFrameData();

    return stackFrames.at(currentIndex);
}

bool StackFrameModel::setData(const QModelIndex &idx, const QVariant &data, int role)
{
    Q_UNUSED(data)
    if (role == ItemActivatedRole || role == ItemClickedRole) {
        setCurrentIndex(idx.row());
        return true;
    }

    return false;
}

void StackFrameModel::setFrames(const StackFrames &frames, bool canExpand)
{
    beginResetModel();
    contentsValid = true;
    this->canExpand = canExpand;
    stackFrames = frames;
    if (stackFrames.size() >= 0)
        setCurrentIndex(0);
    else
        currentIndex = -1;
    endResetModel();
    emit stackChanged();
}

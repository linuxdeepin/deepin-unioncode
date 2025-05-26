// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsmodel.h"
#include "constants.h"
#include "mainframe/environmentview.h"

#include <QUuid>
#include <QIcon>

const Qt::ItemFlags ITEM_FLAGS = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;

BinaryToolsModel::BinaryToolsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

BinaryToolsModel::~BinaryToolsModel()
{
}

int BinaryToolsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant BinaryToolsModel::data(const QModelIndex &index, int role) const
{
    if (auto tool = toolForIndex(index))
        return data(*tool, role);

    bool found;
    QString group = groupForIndex(index, &found);
    if (found)
        return data(group, role);

    return QVariant();
}

QVariant BinaryToolsModel::data(const ToolInfo &tool, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
        return tool.name;
    case Qt::DecorationRole:
        return QIcon::fromTheme(tool.icon);
    default:
        break;
    }

    return QVariant();
}

QVariant BinaryToolsModel::data(const QString &group, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
        return group.isEmpty() ? tr("Default Group") : group;
    default:
        break;
    }

    return QVariant();
}

QModelIndex BinaryToolsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column == 0 && parent.isValid()) {
        bool found;
        QString group = groupForIndex(parent, &found);
        if (found) {
            auto items = binaryTools.value(group);
            if (row < items.count()) {
                auto &item = items.at(row);
                return createIndex(row, 0, const_cast<ToolInfo *>(&item));
            }
        }
    } else if (column == 0 && row < binaryTools.size()) {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex BinaryToolsModel::parent(const QModelIndex &child) const
{
    if (auto tool = toolForIndex(child)) {
        int groupIndex = 0;
        for (const auto &toolsInGroup : binaryTools) {
            if (toolsInGroup.contains(*tool))
                return index(groupIndex, 0);
            ++groupIndex;
        }
    }
    return QModelIndex();
}

int BinaryToolsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return binaryTools.size();

    if (toolForIndex(parent))
        return 0;

    bool found = false;
    QString group = groupForIndex(parent, &found);
    if (found)
        return binaryTools.value(group).count();

    return 0;
}

Qt::ItemFlags BinaryToolsModel::flags(const QModelIndex &index) const
{
    if (toolForIndex(index))
        return ITEM_FLAGS;

    bool found = false;
    groupForIndex(index, &found);
    if (found)
        return ITEM_FLAGS;

    return {};
}

bool BinaryToolsModel::setData(const QModelIndex &modelIndex, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    QString string = value.toString();
    if (auto tool = toolForIndex(modelIndex)) {
        if (string.isEmpty() || tool->name == string)
            return false;

        // rename tool
        tool->name = string;
        emit dataChanged(modelIndex, modelIndex);
        return true;
    } else {
        bool found;
        QString group = groupForIndex(modelIndex, &found);
        if (found) {
            if (string.isEmpty() || binaryTools.contains(string))
                return false;

            // rename group
            QList<QString> groupList = binaryTools.keys();
            int previousIndex = groupList.indexOf(group);
            groupList.removeAt(previousIndex);
            groupList.append(string);
            std::stable_sort(std::begin(groupList), std::end(groupList));
            int newIndex = groupList.indexOf(string);
            if (newIndex != previousIndex) {
                // we have same parent so we have to do special stuff for beginMoveRows...
                int beginMoveRowsSpecialIndex = (previousIndex < newIndex ? newIndex + 1 : newIndex);
                beginMoveRows(QModelIndex(), previousIndex, previousIndex, QModelIndex(), beginMoveRowsSpecialIndex);
            }

            auto items = binaryTools.take(group);
            binaryTools.insert(string, items);
            if (newIndex != previousIndex)
                endMoveRows();
            return true;
        }
    }

    return false;
}

void BinaryToolsModel::setTools(const QMap<QString, QList<ToolInfo>> &tools)
{
    beginResetModel();
    binaryTools = tools;
    endResetModel();
}

ToolInfo *BinaryToolsModel::toolForIndex(const QModelIndex &index) const
{
    auto item = static_cast<ToolInfo *>(index.internalPointer());
    return item;
}

QString BinaryToolsModel::groupForIndex(const QModelIndex &index, bool *found) const
{
    if (index.isValid() && !index.parent().isValid() && index.column() == 0 && index.row() >= 0) {
        const QList<QString> &keys = binaryTools.keys();
        if (index.row() < keys.count()) {
            if (found) *found = true;
            return keys.at(index.row());
        }
    }

    if (found) *found = false;
    return QString();
}

QModelIndex BinaryToolsModel::addGroup()
{
    const QString &groupBase = tr("New Group");
    QString group = groupBase;
    int count = 0;
    while (binaryTools.contains(group)) {
        ++count;
        group = groupBase + QString::number(count);
    }

    QList<QString> groupList = binaryTools.keys();
    groupList.append(group);
    std::stable_sort(std::begin(groupList), std::end(groupList));
    int pos = groupList.indexOf(group);

    beginInsertRows(QModelIndex(), pos, pos);
    binaryTools.insert(group, QList<ToolInfo>());
    endInsertRows();
    return index(pos, 0);
}

QModelIndex BinaryToolsModel::addTool(const QModelIndex &index)
{
    return addTool(false, index);
}

QModelIndex BinaryToolsModel::addCombination(const QModelIndex &index)
{
    return addTool(true, index);
}

void BinaryToolsModel::removeTool(const QModelIndex &index)
{
    auto tool = toolForIndex(index);
    int groupIndex = 0;
    for (auto &items : binaryTools) {
        int pos = items.indexOf(*tool);
        if (pos != -1) {
            beginRemoveRows(this->index(groupIndex, 0), pos, pos);
            items.removeAt(pos);
            endRemoveRows();
            break;
        }
        ++groupIndex;
    }
}

void BinaryToolsModel::removeGroup(const QModelIndex &index)
{
    bool found = false;
    auto group = groupForIndex(index, &found);
    if (!found)
        return;

    beginResetModel();
    binaryTools.remove(group);
    endResetModel();
}

QModelIndex BinaryToolsModel::addTool(bool isCombination, const QModelIndex &index)
{
    bool found;
    QString group = groupForIndex(index, &found);
    if (!found)
        group = groupForIndex(index.parent(), &found);

    ToolInfo tool;
    tool.displyGroup = group;
    tool.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (!isCombination) {
        tool.name = tr("New Tool");
        tool.type = CommandTool;
        tool.description = tr("This tool prints a line of useful text");
        tool.command = "echo";
        tool.arguments = tr("Useful text");
        tool.environment = EnvironmentView::defaultEnvironment();
    } else {
        tool.name = tr("New Combination");
        tool.type = CombinationTool;
    }

    int pos;
    QModelIndex parent;
    if (index.parent().isValid()) {
        pos = index.row() + 1;
        parent = index.parent();
    } else {
        pos = binaryTools.value(group).count();
        parent = index;
    }
    beginInsertRows(parent, pos, pos);
    binaryTools[group].insert(pos, tool);
    endInsertRows();
    return this->index(pos, 0, parent);
}

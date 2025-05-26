// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSMODEL_H
#define BINARYTOOLSMODEL_H

#include "constants.h"

#include <QAbstractItemModel>

class BinaryToolsModel : public QAbstractItemModel
{
public:
    explicit BinaryToolsModel(QObject *parent = nullptr);
    ~BinaryToolsModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &modelIndex, const QVariant &value, int role = Qt::EditRole) override;

    void setTools(const QMap<QString, QList<ToolInfo> > &tools);
    QMap<QString, QList<ToolInfo>> tools() const { return binaryTools; }

    ToolInfo *toolForIndex(const QModelIndex &index) const;
    QString groupForIndex(const QModelIndex &index, bool *found) const;
    QModelIndex addGroup();
    QModelIndex addTool(const QModelIndex &index);
    QModelIndex addCombination(const QModelIndex &index);
    void removeTool(const QModelIndex &index);
    void removeGroup(const QModelIndex &index);

private:
    QModelIndex addTool(bool isCombination, const QModelIndex &index);
    QVariant data(const ToolInfo &tool, int role = Qt::DisplayRole) const;
    QVariant data(const QString &group, int role = Qt::DisplayRole) const;

private:
    QMap<QString, QList<ToolInfo>> binaryTools;
};

#endif   // BINARYTOOLSMODEL_H

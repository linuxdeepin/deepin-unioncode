/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QTreeView>

class Node;
class TreeModelPrivate;
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class TreeView;
    TreeModelPrivate *const d;
public:
    explicit TreeModel(QObject *parent = nullptr);
    virtual ~TreeModel() override;

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QModelIndex index(Node *node);

signals:
    void renamedChild(Node *node);
    void createdChild(Node *node);

private slots: // proxy called slots
    void appendedFile(Node *node);
    void appendedFolder(Node *node);
    void refreshFiles(Node *node);
    void refreshFolders(Node *node);
    void refreshData(Node *node);
    void createdFile(const QString &path);
    void createdFolder(const QString &path);

private slots: // view called slots
    void removeFile(Node *node);
    void removeFolder(Node *node);
    void removeAllFile();
    void removeAllFolder();
};

#endif // TREEMODEL_H

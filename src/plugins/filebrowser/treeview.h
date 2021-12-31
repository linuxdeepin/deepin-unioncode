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
#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

class Node;
class TreeViewPrivate;
class TreeView : public QTreeView
{
    Q_OBJECT
    TreeViewPrivate * const d;
public:
    explicit TreeView(QWidget *parent = nullptr);
    virtual ~TreeView() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void selectNode(Node *node);
    void closeTheFile(Node *node);
    void closeTheFolder(Node *node);
    void closeAll(Node *node);
    void rename(Node *node);
    void doubleClicked(const QModelIndex &index);
};

#endif // TREEVIEW_H

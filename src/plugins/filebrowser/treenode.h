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
#ifndef TREENODE_H
#define TREENODE_H

#include <QSharedData>

class Node
{
public:
    enum Type{Unknown, Files, Folders, File, Folder};

    explicit Node(Node* parent = nullptr,
                  const QString &text = "",
                  const QString &toolTip = "",
                  Type type = Unknown);

    virtual ~Node();
    Node *parent;
    QString text;
    QString toolTip;
    Type type;
    QList<Node*> children;
    bool isLoaded;
};
#endif // TREENODE_H

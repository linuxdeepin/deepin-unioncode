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
#include "treenodewatcher.h"
#include "treenode.h"

#include <QDebug>
#include <QFileSystemWatcher>

class TreeNodeWatcherPrivate
{
    friend class TreeNodeWatcher;
    QFileSystemWatcher fileWatcher;
    QList<Node*> nodes;
};

TreeNodeWatcher::TreeNodeWatcher(QObject *parent)
    : QObject (parent)
    , d(new TreeNodeWatcherPrivate())
{
    QObject::connect(&d->fileWatcher, &QFileSystemWatcher::fileChanged,
                     [=](const QString &path){
        foreach (auto node, d->nodes) {
            if (node->toolTip == path) {
                emit dataChanged(node);
            }
        }
    });

    QObject::connect(&d->fileWatcher, &QFileSystemWatcher::directoryChanged,
                     [=](const QString &path){
        foreach (auto node, d->nodes) {
            if (node->toolTip == path) {
                emit dataChanged(node);
            }
        }
    });
}

TreeNodeWatcher::~TreeNodeWatcher()
{
    if (d)
        delete d;
}

bool TreeNodeWatcher::hasNode(Node *node)
{
    return d->nodes.contains(node);
}

void TreeNodeWatcher::append(Node *node)
{
    if (!node)
        return;

    //添加了当前节点
    if (hasNode(node))
        return;

    //监听目录存在当前目录
    if (d->fileWatcher.files().contains(node->toolTip)
            || d->fileWatcher.directories().contains(node->toolTip))
        return;

    d->nodes.append(node);
    d->fileWatcher.addPath(node->toolTip);
}

void TreeNodeWatcher::remove(Node *node)
{
    if (!node)
        return;
    d->nodes.removeOne(node);
    d->fileWatcher.removePath(node->toolTip);
}

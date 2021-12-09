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

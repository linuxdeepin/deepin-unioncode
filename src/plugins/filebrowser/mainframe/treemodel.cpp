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
#include "treemodel.h"
#include "treeproxy.h"

#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QDirIterator>
#include <QMutex>
#include <QReadLocker>
#include <QFileIconProvider>
#include <QThread>
#include <QApplication>

class TreeModelPrivate
{
    friend class TreeModel;
    QFileIconProvider provider;
    QString changedFilePath;
    QString createFilePath;
};

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel (parent)
    , d(new TreeModelPrivate())
{
    QObject::connect(&TreeProxy::instance(), &TreeProxy::dataChangedFromWatcher,
                     this, &TreeModel::refreshData);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::appendedFile,
                     this, &TreeModel::appendedFile);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::appendedFolder,
                     this, &TreeModel::appendedFolder);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::createdFile,
                     this, &TreeModel::createdFile);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::createdFolder,
                     this, &TreeModel::createdFolder);
}

TreeModel::~TreeModel()
{
    if (d)
        delete d;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row == 0)
            return createIndex(row, column, TreeProxy::instance().files());
        if (row == 1)
            return createIndex(row, column, TreeProxy::instance().folders());
    }

    if (parent.isValid() && parent.internalPointer() == TreeProxy::instance().files()) {
        return createIndex(row, column, TreeProxy::instance().files()->children.at(row));
    }

    if (parent.isValid()) {
        Node* parentNode = static_cast<Node*>(parent.internalPointer());
        if (TreeProxy::instance().root(parentNode)
                == TreeProxy::instance().folders()
                && row < parentNode->children.size()) {
            return createIndex(row, column, parentNode->children.at(row));
        }
    }

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Node *childNode = static_cast<Node*>(child.internalPointer());
    Node *filesRootNode = TreeProxy::instance().files();
    Node *foldersRootNode = TreeProxy::instance().folders();
    if (childNode == filesRootNode || childNode == foldersRootNode)
        return QModelIndex();

    if (childNode && childNode->parent == filesRootNode) {
        return createIndex(0, 0, TreeProxy::instance().files());
    }

    if (childNode && childNode->parent == foldersRootNode) {
        return createIndex(1, 0, TreeProxy::instance().folders());
    }

    if (childNode && childNode->parent) {
        return createIndex(TreeProxy::instance().row(childNode->parent),
                           0, childNode->parent);
    }

    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Node * parentNode = static_cast<Node *>(parent.internalPointer());
        if (parentNode)
            return parentNode->children.size();
    }

    return 2;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

bool TreeModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return true;
    }

    Node* parentNode = static_cast<Node*>(parent.internalPointer());
    if (parentNode) {
        if (parentNode->type == Node::Folder
                || parentNode->type == Node::Folders
                || parentNode->type == Node::Files) {
            return true;
        }
    }
    return false;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    Node* node = static_cast<Node*>(index.internalPointer());
    if (node) {
        if (role == Qt::DisplayRole)
            return node->text;
        if (role == Qt::ToolTipRole)
            return node->toolTip;
        if (role == Qt::DecorationRole) {
            if(node->type == Node::File)
                return d->provider.icon(QFileIconProvider::File);
            if(node->type == Node::Folder)
                return d->provider.icon(QFileIconProvider::Folder);
        }
    }
    return QVariant();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);

    if (!index.isValid()) {
        return false;
    }

    QString changedText = value.toString();
    if (changedText.isEmpty())
        return false;
    if (changedText == ".")
        return false;
    if (changedText == "..")
        return false;

    Node* node = static_cast<Node*>(index.internalPointer());
    if (!node)
        return false;

    QFileInfo info(node->toolTip);

    d->changedFilePath = info.path() + QDir::separator() + changedText;
    bool ret = QFile(node->toolTip).rename(d->changedFilePath);

    return ret;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        Node* node = static_cast<Node*>(index.internalPointer());
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        // 设置可编辑
        if (node->parent != TreeProxy::instance().files()
                && node->parent != TreeProxy::instance().folders()) {
            flags = flags | Qt::ItemIsEditable;
        }
        return flags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex TreeModel::index(Node *node)
{
    if (!node)
        return QModelIndex();

    if (node == TreeProxy::instance().files()) {
        return index(0, 0);
    }

    if (node == TreeProxy::instance().folders()) {
        return index(1, 0);
    }

    if (node->parent == TreeProxy::instance().files()) {
        int nodeRow = node->parent->children.indexOf(node);
        return index(nodeRow, 0, index(0, 0)) ;
    }

    if (TreeProxy::instance().root(node)
            == TreeProxy::instance().folders()) {
        QModelIndex parentIndex = index(1, 0); //根index
        QModelIndex nodeIndex;
        //获取层级路径
        auto parentRows = TreeProxy::instance().parentsRowFind(node);
        for (int idx = 0; idx < parentRows.size(); idx++) { //寻路
            int row = parentRows.at(idx); //层级行
            nodeIndex = index(row, 0, parentIndex); //获取子节点index
            //当前index是 寻找的node
            if (node == static_cast<Node*>(nodeIndex.internalPointer()))
                return nodeIndex;
            parentIndex = nodeIndex;
        }
    }
    return QModelIndex();
}

void TreeModel::appendedFile(Node *node)
{
    if (!node)
        return;

    int appededIndex = TreeProxy::instance().files()->children.indexOf(node);
    if (0 > appededIndex)
        return;

    beginInsertRows(index(0,0), appededIndex, appededIndex);
    endInsertRows();
}

void TreeModel::appendedFolder(Node *node)
{
    if (!node)
        return;

    int appededIndex = TreeProxy::instance().folders()->children.indexOf(node);
    if (0 > appededIndex)
        return;

    beginInsertRows(index(1,0), appededIndex, appededIndex);
    endInsertRows();
}

void TreeModel::removeFile(Node *node)
{
    if (!node)
        return;

    int rmIdx = TreeProxy::instance().files()->children.indexOf(node);
    if (rmIdx < 0)
        return;

    beginRemoveRows(index(0, 0), rmIdx, rmIdx);
    TreeProxy::instance().files()->children.removeAt(rmIdx);
    TreeProxy::instance().removeWatcherNode(node); //从监听中删除
    delete node;
    endRemoveRows();
}

void TreeModel::removeFolder(Node *node)
{
    if (!node)
        return;

    int rmIdx = TreeProxy::instance().folders()->children.indexOf(node);
    if (rmIdx < 0)
        return;

    beginRemoveRows(index(1, 0), rmIdx, rmIdx);
    TreeProxy::instance().folders()->children.removeAt(rmIdx);
    TreeProxy::instance().removeWatcherNode(node); //从监听中删除
    delete node;
    endRemoveRows();
}

void TreeModel::removeAllFile()
{
    Node *files = TreeProxy::instance().files();
    int childCount = files->children.size();
    beginRemoveRows(index(0, 0), 0, childCount);
    for (int idx = 0; idx < childCount; idx++) {
        auto child = files->children.at(idx);
        TreeProxy::instance().removeWatcherNode(child); //从监听中删除
        delete child; //释放节点
    }
    files->children.clear(); //清除索引
    endRemoveRows();
}

void TreeModel::removeAllFolder()
{
    Node *folders = TreeProxy::instance().folders();
    int childCount = folders->children.size();
    beginRemoveRows(index(1, 0), 0, childCount);
    for (int idx = 0; idx < childCount; idx++) {
        auto child = folders->children.at(idx);
        TreeProxy::instance().removeWatcherNode(child); //从监听中删除
        delete child; //释放节点
    }
    folders->children.clear(); //清除索引
    endRemoveRows();
}

void TreeModel::refreshFiles(Node *node)
{
    Node *files = TreeProxy::instance().files();
    if (!node || TreeProxy::instance().root(node) != files)
        return;

    int nodeRow = files->children.indexOf(node);
    if (node->type == Node::File
            && node->parent
            && !QFileInfo(node->toolTip).exists()) {
        beginRemoveRows(index(files), nodeRow, nodeRow);
        node->parent->children.removeOne(node); //从节点删除
        endRemoveRows();
        TreeProxy::instance().removeWatcherNode(node); //从监听中删除
        delete node; //释放内存
    }
}

void TreeModel::refreshFolders(Node *node)
{
    Node *folders = TreeProxy::instance().folders();
    if (!node || TreeProxy::instance().root(node) != folders
            || !node->isLoaded)
        return;

    QDir dir(node->toolTip);
    auto dirInfoChilds = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs,
                                           QDir::DirsFirst|QDir::Name);
    qInfo() << node->toolTip;
    int insertIndex = 0; //遍历时计算插入的索引
    foreach(auto dirChild, dirInfoChilds) {
        //避免重复项加入
        if (!TreeProxy::instance().hasChild(node, dirChild.fileName(), dirChild.filePath())) {
            qInfo() << dirChild.fileName() << dirChild.filePath();
            //创建新的节点
            Node *child = new Node(node);
            child->text = dirChild.fileName();
            child->toolTip = dirChild.filePath();
            if (dirChild.isDir()) child->type = Node::Folder;
            if (dirChild.isFile()) child->type = Node::File;
            beginInsertRows(index(node), insertIndex, insertIndex);
            node->children.insert(insertIndex, child); //添加到树节点上
            endInsertRows();
            //重命名结果则发送
            if (child->toolTip == d->changedFilePath) {
                emit renamedChild(child);
                d->changedFilePath.clear();
            }
            //新建结果则发送
            if (child->toolTip == d->createFilePath) {
                emit createdChild(child);
                d->createFilePath.clear();
            }
        }
        insertIndex ++;
    }

    foreach(auto dirChild, node->children) {
        if (!QFileInfo(dirChild->toolTip).exists()) {
            node->children.removeOne(dirChild); //从树节点删除
            beginRemoveRows(index(node), node->children.indexOf(dirChild),
                            node->children.indexOf(dirChild));
            endRemoveRows();
            TreeProxy::instance().removeWatcherNode(dirChild); //从监听中删除
            delete dirChild; //释放内存
        }
    }
}

void TreeModel::refreshData(Node *node)
{
    refreshFiles(node);
    refreshFolders(node);
}

void TreeModel::createdFile(const QString &path)
{
    if (path.isEmpty())
        return;
    d->createFilePath = path;
}

void TreeModel::createdFolder(const QString &path)
{
    if (path.isEmpty())
        return;
    d->createFilePath = path;
}

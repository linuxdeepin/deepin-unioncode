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
#include "treeproxy.h"
#include "treenodewatcher.h"

#include "common/common.h"

#include <QMutex>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QModelIndex>
#include <QFileSystemWatcher>
#include <QThread>
#include <QProcess>
#include <QMessageBox>
#include <QAbstractButton>

const QString FILES {TreeProxy::tr("Open Document")};
const QString FILES_TOOLTIP {TreeProxy::tr("Open Document")};

const QString FOLDERS {TreeProxy::tr("Open Folder")};
const QString FOLDERS_TOOLTIP {TreeProxy::tr("Open Folder")};

const QString NEW_FILE_NAME = "NewFile.txt";
const QString NEW_FOLDER_NAME = "NewFolder";

const QString DELETE_MESSAGE_TEXT {TreeProxy::tr("The delete operation will be removed from"
                                                 "the disk and will not be recoverable "
                                                 "after this operation.\nDelete anyway?")};

const QString DELETE_WINDOW_TEXT {TreeProxy::tr("Delete Warning")};

class TreeProxyPrivate
{
    friend class TreeProxy;
    TreeProxy * const q;
    TreeNodeWatcher nodeWatcher;
    TreeProxyPrivate(TreeProxy * qq):q(qq){}
    Node iFiles {nullptr, FILES, FILES_TOOLTIP, Node::Folders};
    Node iFolders {nullptr, FOLDERS, FOLDERS_TOOLTIP, Node::Folders};
    Node *files() {return &iFiles;}
    Node *folders() {return &iFolders;}
};

TreeProxy::TreeProxy(QObject *parent)
    : QObject (parent)
    , d(new TreeProxyPrivate(this))
{
    QObject::connect(&d->nodeWatcher, &TreeNodeWatcher::dataChanged,
                     this, &TreeProxy::dataChangedFromWatcher);
}

TreeProxy &TreeProxy::instance()
{
    static TreeProxy proxy;
    return proxy;
}

Node* TreeProxy::files() const
{
    return d->files();
}

Node* TreeProxy::folders() const
{
    return d->folders();
}

Node *TreeProxy::root(Node *node) const
{
    Node *temp = node;
    while (temp->parent) {
        temp = temp->parent;
    }
    return temp;
}

Node *TreeProxy::rootFromFolder(Node *node) const
{
    Node *temp = node;
    while (temp->parent != folders()) {
        temp = temp->parent;
    }
    return temp;
}

Node *TreeProxy::rootFromFiles(Node *node) const
{
    Node *temp = node;
    while (temp->parent != files()) {
        temp = temp->parent;
    }
    return temp;
}

void TreeProxy::loadChildren(Node *node)
{
    Node *folders = TreeProxy::instance().folders();
    if (!node || root(node) != folders)
        return;

    QDir dir(node->toolTip);
    auto dirInfoChilds = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs,
                                           QDir::DirsFirst|QDir::Name);
    foreach(auto dirChild, dirInfoChilds) {
        //避免重复项加入
        if (!hasChild(node, dirChild.fileName(), dirChild.filePath())) {
            Node *child = new Node(node); //创建新的节点
            child->text = dirChild.fileName();
            child->toolTip = dirChild.filePath();
            if (dirChild.isDir()) child->type = Node::Folder;
            if (dirChild.isFile()) child->type = Node::File;
            node->children.append(child); //添加到树节点上
        }
    }

    foreach(auto dirChild, node->children) {
        if (!QFileInfo(dirChild->toolTip).exists()) {
            node->children.removeOne(dirChild); //从树节点删除
            delete dirChild; //释放内存
        }
    }

    appendWatcherNode(node); // 当前节点添加监听
}

QList<int> TreeProxy::parentsRowFind(Node *node) const
{
    Node *temp = node;
    QList<int> ret;
    while (temp->parent) {
        ret.push_front(temp->parent->children.indexOf(temp));
        temp = temp->parent;
    }
    return ret;
}

bool TreeProxy::hasChild(Node *node, const QString &text, const QString &toolTip) const
{
    if (!node) return node;
    for (auto val : node->children) {
        if (val->text == text && val->toolTip == toolTip) {
            return true;
        }
    }
    return false;
}

int TreeProxy::row(Node *node)
{
    return node->parent->children.indexOf(node);
}

void TreeProxy::appendWatcherNode(Node *node)
{
    d->nodeWatcher.append(node); // 添加watcher
}

void TreeProxy::removeWatcherNode(Node *node)
{
    d->nodeWatcher.remove(node);
}

TreeProxy::~TreeProxy()
{
    if (d)
        delete d;
}

void TreeProxy::appendFile(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return;
    }

    if(hasChild(d->files(), info.fileName(), info.filePath()))
        return;

    Node *newFileNode = new Node { d->files(),
            info.fileName(),info.filePath(), Node::File };
    d->files()->children.append(newFileNode); // 添加子节点
    appendWatcherNode(newFileNode);
    emit appendedFile(newFileNode);
}

void TreeProxy::appendFolder(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        return;
    }

    if(hasChild(d->folders(), info.fileName(), info.filePath()))
        return;

    Node *newFolderNode = new Node {
            d->folders(), info.fileName(),
            info.filePath(), Node::Folder};
    d->folders()->children.append(newFolderNode); // 添加子节点
    appendWatcherNode(newFolderNode);
    emit appendedFolder(newFolderNode);
}

void TreeProxy::createFile(const QString &path)
{
    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    int index = 1;
    QString newFileName = parentInfo.filePath() + QDir::separator() + NEW_FILE_NAME;
    QFileInfo createFileInfo(newFileName);
    while(createFileInfo.exists()) {
        createFileInfo.setFile(newFileName + QString("(%0)").arg(index));
        index ++;
    }
    bool result = QFile(createFileInfo.filePath()).open(QFile::Text|QFile::WriteOnly);
    if (result)
        emit createdFile(createFileInfo.filePath());
}

void TreeProxy::createFolder(const QString &path)
{
    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    int index = 1;
    QString newFolderName = parentInfo.filePath() + QDir::separator() + NEW_FOLDER_NAME;
    QFileInfo createFolderInfo(newFolderName);
    while(createFolderInfo.exists()) {
        createFolderInfo.setFile(newFolderName + QString("(%0)").arg(index));
        index ++;
    }
    bool result = QDir().mkpath(createFolderInfo.filePath());
    if (result)
        emit createdFolder(createFolderInfo.filePath());
}

void TreeProxy::deleteFile(const QString &path)
{
    auto okCallBack = [=](bool checked) {
        Q_UNUSED(checked)
        QFile(path).remove();
    };

    ContextDialog::okCancel(DELETE_MESSAGE_TEXT,
                            DELETE_WINDOW_TEXT,
                            QMessageBox::Warning,
                            okCallBack,
                            nullptr);
}

void TreeProxy::deleteFolder(const QString &path)
{
    auto okCallBack = [=](bool checked) {
        Q_UNUSED(checked)
        QDir(path).removeRecursively();
    };

    ContextDialog::okCancel(DELETE_MESSAGE_TEXT,
                            DELETE_WINDOW_TEXT,
                            QMessageBox::Warning,
                            okCallBack,
                            nullptr);
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directoryasynparse.h"
#include "projectgenerator.h"

#include "common/common.h"
#include "services/project/projectservice.h"

#include <QAction>

using namespace std::placeholders;

class DirectoryAsynParsePrivate
{
    friend class DirectoryAsynParse;
    QString rootPath;
    QList<QStandardItem *> itemList;
    QSet<QString> fileList {};
};

DirectoryAsynParse::DirectoryAsynParse()
    : d(new DirectoryAsynParsePrivate)
{
    QObject::connect(this, &QFileSystemWatcher::directoryChanged,
                     this, &DirectoryAsynParse::doDirectoryChanged);
}

DirectoryAsynParse::~DirectoryAsynParse()
{
    if (d) {
        delete d;
    }
}

void DirectoryAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.workspaceFolder());
    emit itemsCreated(d->itemList);
}

QSet<QString> DirectoryAsynParse::getFilelist()
{
    return d->fileList;
}

void DirectoryAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    // auto item = findItem(path);
    // if (!item)
    //     item = d->rootItem;
    // updateItem(item);
    Q_EMIT reqUpdateItem(path);
}

void DirectoryAsynParse::createRows(const QString &path)
{
    QString rootPath = path;
    d->fileList.clear();
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    // 缓存当前工程目录
    d->rootPath = rootPath;
    QFileSystemWatcher::addPath(d->rootPath);

    QDir dir;
    dir.setPath(rootPath);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    QDirIterator iter(dir, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        iter.next();
        QStandardItem *parent = findParentItem(iter.filePath());
        auto item = new QStandardItem(iter.fileName());
        item->setData(iter.filePath(), Project::FileIconRole);
        item->setData(iter.filePath(), Project::FilePathRole);
        item->setToolTip(iter.filePath());
        parent ? parent->appendRow(item) : d->itemList.append(item);
        if (iter.fileInfo().isFile())
            d->fileList.insert(iter.filePath());
        else
            QFileSystemWatcher::addPath(iter.filePath());
    }

    sortItems();
}

QList<QStandardItem *> DirectoryAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QList<QStandardItem *> DirectoryAsynParse::takeAll(QStandardItem *item)
{
    QList<QStandardItem *> itemList;
    for (int i = 0; i < item->rowCount(); ++i) {
        itemList.append(item->takeChild(i));
    }

    item->removeRows(0, item->rowCount());
    return itemList;
}

QStandardItem *DirectoryAsynParse::findItem(const QString &path, QStandardItem *parent) const
{
    if (path.isEmpty())
        return nullptr;

    const auto &itemRows = parent ? rows(parent) : d->itemList;
    for (const auto item : itemRows) {
        const auto &itemPath = item->data(Project::FilePathRole).toString();
        if (path == itemPath) {
            return item;
        }

        if (path.startsWith(itemPath) && item->hasChildren()) {
            if (auto *found = findItem(path, item))
                return found;
        }
    }

    return nullptr;
}

QStandardItem *DirectoryAsynParse::findParentItem(const QString &path, QStandardItem *parent) const
{
    if (path.isEmpty())
        return nullptr;

    QFileInfo fileInfo(path);
    const auto &itemRows = parent ? rows(parent) : d->itemList;
    for (const auto item : itemRows) {
        const auto &itemPath = item->data(Project::FilePathRole).toString();
        if (!path.startsWith(itemPath))
            continue;

        if (fileInfo.absolutePath() == itemPath)
            return item;

        if (item->hasChildren()) {
            if (auto *found = findParentItem(path, item))
                return found;
        }
    }
    return nullptr;
}

void DirectoryAsynParse::updateItem(QStandardItem *item)
{
    if (!item)
        return;

    const QString path = item->data(Project::FilePathRole).toString();
    const QDir dir(path);
    if (!dir.exists())
        return;

    QStringList tempFileList = d->fileList.values();
    QStringList existingPaths;
    // Remove non-existent items
    for (int i = item->rowCount() - 1; i >= 0; --i) {
        if (QStandardItem *child = item->child(i)) {
            const auto filePath = child->data(Project::FilePathRole).toString();
            if (!QFile::exists(filePath)) {
                item->removeRow(i);
                bool isDir = QFileInfo(filePath).isDir();
                auto iter = std::remove_if(tempFileList.begin(), tempFileList.end(),
                                           [&isDir, &filePath](const QString &file) {
                                               if (isDir)
                                                   return file.startsWith(filePath);
                                               else
                                                   return file == filePath;
                                           });
                tempFileList.erase(iter, tempFileList.end());
            } else {
                existingPaths << filePath;
            }
        }
    }

    for (auto tempFile : tempFileList)
        d->fileList.insert(tempFile);
    bool hasAdded = false;
    // Process directory entries
    const QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : entries) {
        const QString filePath = info.filePath();
        // Add new items if they don't exist
        if (!existingPaths.contains(filePath)) {
            hasAdded = true;
            QStandardItem *newItem = new QStandardItem(info.fileName());
            newItem->setData(filePath, Project::FileIconRole);
            newItem->setData(filePath, Project::FilePathRole);
            newItem->setToolTip(filePath);
            item->appendRow(newItem);
            if (info.isFile())
                d->fileList.insert(filePath);
            else
                QFileSystemWatcher::addPath(filePath);
        }
    }

    if (hasAdded)
        sortChildren(item);
}

void DirectoryAsynParse::sortItems()
{
    // Sort root level items
    std::sort(d->itemList.begin(), d->itemList.end(), std::bind(&DirectoryAsynParse::compareItems, this, _1, _2));

    // Sort children of each root item
    for (auto *item : d->itemList) {
        sortChildren(item);
    }
}

void DirectoryAsynParse::sortChildren(QStandardItem *parentItem)
{
    if (!parentItem)
        return;

    auto children = takeAll(parentItem);
    if (children.isEmpty())
        return;

    // Sort current level
    std::sort(children.begin(), children.end(), std::bind(&DirectoryAsynParse::compareItems, this, _1, _2));
    for (auto *child : children) {
        parentItem->appendRow(child);
        // Recursively sort children
        sortChildren(child);
    }
}

bool DirectoryAsynParse::compareItems(QStandardItem *item1, QStandardItem *item2)
{
    QString path1 = item1->data(Project::FilePathRole).toString();
    QString path2 = item2->data(Project::FilePathRole).toString();

    QFileInfo info1(path1);
    QFileInfo info2(path2);

    // If one is directory and other is file, directory comes first
    if (info1.isDir() != info2.isDir())
        return info1.isDir();

    // If both are same type (files or directories), sort by name
    return path1.toLower() < path2.toLower();
}

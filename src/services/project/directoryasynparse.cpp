// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directoryasynparse.h"
#include "projectgenerator.h"

#include "common/common.h"
#include "services/project/projectservice.h"

#include <QAction>

class DirectoryAsynParsePrivate
{
    friend class DirectoryAsynParse;
    QString rootPath;
    QSet<QString> fileList {};
    QList<QStandardItem *> rows {};
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
    emit itemsModified(d->rows);
}

QSet<QString> DirectoryAsynParse::getFilelist()
{
    return d->fileList;
}

void DirectoryAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    d->rows.clear();

    createRows(d->rootPath);

    emit itemsModified(d->rows);
}

QString DirectoryAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
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

    {   // 避免变量冲突 迭代文件夹
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        dir.setSorting(QDir::Name);
        QDirIterator dirItera(dir, QDirIterator::Subdirectories);
        while (dirItera.hasNext()) {
            QString childPath = dirItera.next().remove(0, rootPath.size());
            QFileSystemWatcher::addPath(dirItera.filePath());
            QStandardItem *item = findItem(childPath);
            auto newItem = new QStandardItem(dirItera.fileName());
            newItem->setData(dirItera.filePath(), ProjectItemRole::FileIconRole);
            newItem->setToolTip(dirItera.filePath());
            if (!item) {
                d->rows.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
    {   // 避免变量冲突 迭代文件
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Name);
        QDirIterator fileItera(dir, QDirIterator::Subdirectories);
        while (fileItera.hasNext()) {
            QString childPath = fileItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(childPath);
            auto newItem = new QStandardItem(fileItera.fileName());
            newItem->setData(fileItera.filePath(), ProjectItemRole::FileIconRole);
            newItem->setToolTip(fileItera.filePath());
            if (!item) {
                d->rows.append(newItem);
            } else {
                item->appendRow(newItem);
            }
            d->fileList.insert(fileItera.filePath());
        }
    }
}

QList<QStandardItem *> DirectoryAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QStandardItem *DirectoryAsynParse::findItem(const QString &path,
                                            QStandardItem *parent) const
{
    QString pathTemp = path;
    if (pathTemp.endsWith(QDir::separator())) {
        pathTemp = pathTemp.remove(pathTemp.size() - separatorSize(), separatorSize());
    }

    if (pathTemp.startsWith(QDir::separator()))
        pathTemp.remove(0, separatorSize());

    if (pathTemp.endsWith(QDir::separator()))
        pathTemp.remove(pathTemp.size() - separatorSize(), separatorSize());

    if (pathTemp.isEmpty())
        return parent;

    QStringList splitPaths = pathTemp.split(QDir::separator());
    QString name = splitPaths.takeFirst();

    QList<QStandardItem *> currRows {};
    if (parent) {
        currRows = rows(parent);
    } else {
        currRows = d->rows;
    }

    for (int i = 0; i < currRows.size(); i++) {
        QStandardItem *child = currRows[i];
        if (name == itemDisplayName(child)) {
            if (splitPaths.isEmpty()) {
                return child;
            } else {
                return findItem(splitPaths.join(QDir::separator()), child);
            }
        }
    }
    return parent;
}

int DirectoryAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

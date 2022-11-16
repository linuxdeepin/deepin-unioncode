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
#include "gradleasynparse.h"
#include "gradleitemkeeper.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

class GradleAsynParsePrivate
{
    friend  class GradleAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

GradleAsynParse::GradleAsynParse()
    : d(new GradleAsynParsePrivate)
{
    d->thread = new QThread();
    this->moveToThread(d->thread);

    QObject::connect(this, &Inotify::modified,
                     this, &GradleAsynParse::doDirWatchModify,
                     Qt::DirectConnection);
    QObject::connect(this, &Inotify::createdSub,
                     this, &GradleAsynParse::doWatchCreatedSub,
                     Qt::DirectConnection);
    QObject::connect(this, &Inotify::deletedSub,
                     this, &GradleAsynParse::doWatchDeletedSub,
                     Qt::DirectConnection);
}

GradleAsynParse::~GradleAsynParse()
{
    removeRows();

    if (d) {
        if (d->thread) {
            if (d->thread->isRunning())
                d->thread->quit();
            while (d->thread->isFinished());
            delete d->thread;
            d->thread = nullptr;
        }
        delete d;
    }
}

void GradleAsynParse::loadPoms(const dpfservice::ProjectInfo &info)
{
    QFile docFile(info.projectFilePath());

    if (!docFile.exists()) {
        parsedError({"Failed, maven pro not exists!: " + docFile.fileName(), false});
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {;
        parsedError({docFile.errorString(), false});
    }

    if (!d->xmlDoc.setContent(&docFile)) {
        docFile.close();
    }
    docFile.close();
}

void GradleAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.sourceFolder());
    emit itemsModified({d->rows, true});
}

void GradleAsynParse::doDirWatchModify(const QString &path)
{
    Q_UNUSED(path)
}

void GradleAsynParse::doWatchCreatedSub(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    QString pathTemp = path;
    if (pathTemp.startsWith(d->rootPath))
        pathTemp = pathTemp.remove(0, d->rootPath.size());

    QStringList createds = createdFileNames(path);
    for (auto &created : createds) {
        QStandardItem *item = findItem(pathTemp);
        QString createdFilePath = path + QDir::separator() + created;
        QStandardItem *createItem = new QStandardItem(created);
        createItem->setIcon(CustomIcons::icon(createdFilePath));
        createItem->setToolTip(createdFilePath);

        // add watcher
        if (QFileInfo(createdFilePath).isDir())
            Inotify::addPath(createdFilePath);

        if (item) {
            int insertRow = findRowWithDisplay(rows(item), created);
            if (insertRow >= 0) {
                item->insertRow(insertRow, createItem);
            } else {
                item->appendRow(createItem);
            }
        } else {
            int insertRow = findRowWithDisplay(d->rows, created);
            if (insertRow >= 0) {
                d->rows.insert(insertRow, createItem);
            } else {
                d->rows.append(createItem);
            }
        }
    }

    emit itemsModified({d->rows, true});
}

void GradleAsynParse::doWatchDeletedSub(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    QString pathTemp = path;
    if (pathTemp.startsWith(d->rootPath))
        pathTemp = pathTemp.remove(0, d->rootPath.size());

    QStringList deleteds = deletedFileNames(path);
    for (QString deleted : deleteds) {
        QStandardItem *currPathItem = findItem(pathTemp);
        if (currPathItem) {
            for (int i = 0; i < currPathItem->rowCount(); i++) {
                QStandardItem *child = currPathItem->child(i);
                if (itemDisplayName(child) == deleted) {
                    currPathItem->removeRow(i);
                }
            }
        } else {
            for (int i = 0; i < d->rows.size(); i++) {
                if (itemDisplayName(d->rows[i]) == deleted) {
                    d->rows.removeAt(i);
                }
            }
        }
    }

    emit itemsModified({d->rows, true});
}

QString GradleAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

bool GradleAsynParse::isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role) const
{
    if (!t1 || !t2)
        return false;
    return t1->data(role) == t2->data(role);
}
void GradleAsynParse::createRows(const QString &path)
{
    QString rootPath = path;
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    // 缓存当前工程目录
    d->rootPath = rootPath;
    Inotify::addPath(d->rootPath);

    {// 避免变量冲突 迭代文件夹
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        dir.setSorting(QDir::Name);
        QDirIterator dirItera(dir, QDirIterator::Subdirectories);
        while (dirItera.hasNext()) {
            QString childPath = dirItera.next().remove(0, rootPath.size());
            Inotify::addPath(dirItera.filePath());
            QStandardItem *item = findItem(childPath);
            QIcon icon = CustomIcons::icon(dirItera.fileInfo());
            auto newItem = new QStandardItem(icon, dirItera.fileName());
            newItem->setToolTip(dirItera.filePath());
            if (!item) {
                d->rows.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
    {// 避免变量冲突 迭代文件
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Name);
        QDirIterator fileItera(dir, QDirIterator::Subdirectories);
        while (fileItera.hasNext()) {
            QString childPath = fileItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(childPath);
            QIcon icon = CustomIcons::icon(fileItera.fileInfo());
            auto newItem = new QStandardItem(icon, fileItera.fileName());
            newItem->setToolTip(fileItera.filePath());
            if (!item) {
                d->rows.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
}

void GradleAsynParse::removeRows()
{
    d->rootPath.clear();
    for (int i = 0; i < d->rows.size(); i++) {
        removeSelfSubWatch(d->rows[i]);
    }
    d->rows.clear();
    emit itemsModified({d->rows, true});
}

void GradleAsynParse::removeSelfSubWatch(QStandardItem *item)
{
    if (!item)
        return;

    Inotify::removePath(item->toolTip());

    for (int i = 0; i < item->rowCount(); i++) {
        QStandardItem *child = item->child(i);
        if (!child)
            continue;

        if (QFileInfo(child->toolTip()).isDir()) {
            Inotify::removePath(child->toolTip());
        }

        if (child->hasChildren())
            removeSelfSubWatch(child);
        item->removeRow(i);
    }
}

QList<QStandardItem *> GradleAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

int GradleAsynParse::findRowWithDisplay(QList<QStandardItem *> rowList, const QString &fileName)
{
    if (fileName.isEmpty() || rowList.size() <= 0)
        return -1;

    QString currPath = rowList.first()->toolTip();
    QFileInfo currInfo(currPath);
    if (currInfo.isDir()) {
        int removeCount = currInfo.fileName().size();
        currInfo = currPath.remove(currPath.size() - removeCount, removeCount);
    }
    QFileInfo infoFile(currPath + fileName);

    for (int i = 0; i < rowList.size(); i++) {
        QString name = itemDisplayName(rowList[i]);
        QStandardItem *child = rowList[i];
        if (name.isEmpty())
            continue;

        if (infoFile.isDir() && itemIsDir(child) == infoFile.isDir()) {
            if (*name.begin() >= *fileName.begin()) {
                return i;
            }
        } else if (infoFile.isFile() && itemIsFile(child) == infoFile.isFile()) {
            if (*name.begin() < *fileName.begin()) {
                return i;
            }
        } else if (infoFile.isDir() && itemIsFile(child)) {
            return i;
        }
    }

    return -1;
}

QStandardItem *GradleAsynParse::findItem(const QString &path,
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

    QList<QStandardItem*> currRows{};
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

bool GradleAsynParse::itemIsDir(const QStandardItem *item) const
{
    return QFileInfo(item->toolTip()).isDir();
}

bool GradleAsynParse::itemIsFile(const QStandardItem *item) const
{
    return QFileInfo(item->toolTip()).isFile();
}

int GradleAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

QStringList GradleAsynParse::pathChildFileNames(const QString &path) const
{
    QStringList result;
    QDir dir;
    dir.setPath(path);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::Name);
    QDirIterator dirItera(dir, QDirIterator::NoIteratorFlags);
    while (dirItera.hasNext()) {
        dirItera.next();
        result << dirItera.fileName();
    }
    return result;
}

QStringList GradleAsynParse::createdFileNames(const QString &path) const
{
    QString pathTemp = path;
    if (pathTemp.startsWith(d->rootPath))
        pathTemp = pathTemp.remove(0, d->rootPath.size());

    auto item = findItem(pathTemp);
    QSet<QString> childDisplays;
    QSet<QString> currentDisplays = pathChildFileNames(path).toSet();
    if (!item) {
        childDisplays = displayNames(d->rows).toSet();
    } else {
        childDisplays = displayNames(rows(item)).toSet();
    }
    return (currentDisplays- childDisplays).toList();
}

QStringList GradleAsynParse::displayNames(const QList<QStandardItem *> items) const
{
    QStringList names;
    for (int i = 0; i < items.size(); i++) {
        names << items[i]->data(Qt::DisplayRole).toString();
    }
    return names;
}

QStringList GradleAsynParse::deletedFileNames(const QString &path) const
{
    QString pathTemp = path;
    if (pathTemp.startsWith(d->rootPath))
        pathTemp = pathTemp.remove(0, d->rootPath.size());

    auto item = findItem(pathTemp);
    QSet<QString> childDisplays;
    if (!item) {
        childDisplays = displayNames(d->rows).toSet();
    } else {
        childDisplays = displayNames(rows(item)).toSet();
    }
    return (childDisplays - pathChildFileNames(path).toSet()).toList();
}

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
#include "ninjaasynparse.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

class NinjaAsynParsePrivate
{
    friend  class NinjaAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

NinjaAsynParse::NinjaAsynParse()
    : d(new NinjaAsynParsePrivate)
{
    QObject::connect(this, &QFileSystemWatcher::directoryChanged,
                     this, &NinjaAsynParse::doDirectoryChanged);

    d->thread = new QThread();
    this->moveToThread(d->thread);
    d->thread->start();
}

NinjaAsynParse::~NinjaAsynParse()
{
    if (d) {
        if (d->thread) {
            if (d->thread->isRunning())
                d->thread->quit();
            d->thread->wait();
            d->thread->deleteLater();
            d->thread = nullptr;
        }
        delete d;
    }
}

void NinjaAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.workspaceFolder());
    emit itemsModified(d->rows);
}

void NinjaAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    d->rows.clear();

    createRows(d->rootPath);

    emit itemsModified(d->rows);
}

QString NinjaAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

void NinjaAsynParse::createRows(const QString &path)
{
    QString rootPath = path;
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    // 缓存当前工程目录
    d->rootPath = rootPath;
    QFileSystemWatcher::addPath(d->rootPath);

    {// 避免变量冲突 迭代文件夹
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        dir.setSorting(QDir::Name);
        QDirIterator dirItera(dir, QDirIterator::Subdirectories);
        while (dirItera.hasNext()) {
            QString childPath = dirItera.next().remove(0, rootPath.size());
            QFileSystemWatcher::addPath(dirItera.filePath());
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

QList<QStandardItem *> NinjaAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QStandardItem *NinjaAsynParse::findItem(const QString &path,
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

int NinjaAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

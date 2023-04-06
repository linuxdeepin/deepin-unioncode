/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "jsasynparse.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

class JSAsynParsePrivate
{
    friend  class JSAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

JSAsynParse::JSAsynParse()
    : d(new JSAsynParsePrivate)
{
    QObject::connect(this, &QFileSystemWatcher::directoryChanged,
                     this, &JSAsynParse::doDirectoryChanged);
    d->thread = new QThread();
    this->moveToThread(d->thread);
    d->thread->start();
}

JSAsynParse::~JSAsynParse()
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

void JSAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.workspaceFolder());
    emit itemsModified(d->rows);
}

void JSAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    d->rows.clear();

    createRows(d->rootPath);

    emit itemsModified(d->rows);
}

QString JSAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

void JSAsynParse::iteratorDirectory(const QString &rootPath)
{
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

void JSAsynParse::iteratorFiles(const QString &rootPath)
{
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

void JSAsynParse::createRows(const QString &path)
{
    QString rootPath = path;
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }
    d->rootPath = rootPath;
    QFileSystemWatcher::addPath(d->rootPath);

    iteratorDirectory(rootPath);
    iteratorFiles(rootPath);
}

QList<QStandardItem *> JSAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QStandardItem *JSAsynParse::findItem(const QString &path,
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

int JSAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

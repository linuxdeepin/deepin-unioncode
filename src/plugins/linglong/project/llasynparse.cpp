// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llasynparse.h"

#include <QtXml>

class LLAsynParsePrivate
{
    friend class LLAsynParse;
    QDomDocument xmlDoc;
    QThread *thread { nullptr };
    QString rootPath;
    QSet<QString> fileList {};
    QList<QStandardItem *> rows {};
};

LLAsynParse::LLAsynParse()
    : d(new LLAsynParsePrivate)
{
    QObject::connect(this, &QFileSystemWatcher::directoryChanged,
                     this, &LLAsynParse::doDirectoryChanged);
    d->thread = new QThread();
    this->moveToThread(d->thread);
    d->thread->start();
}

LLAsynParse::~LLAsynParse()
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

void LLAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.workspaceFolder());
    emit itemsModified(d->rows);
}

QSet<QString> LLAsynParse::getFilelist()
{
    return d->fileList;
}

void LLAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    d->rows.clear();

    createRows(d->rootPath);

    emit itemsModified(d->rows);
}

QString LLAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

void LLAsynParse::createRows(const QString &path)
{
    QString rootPath = path;
    d->fileList.clear();
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    d->rootPath = rootPath;
    QFileSystemWatcher::addPath(d->rootPath);

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
            d->fileList.insert(fileItera.filePath());
        }
    }
}

QList<QStandardItem *> LLAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QStandardItem *LLAsynParse::findItem(const QString &path,
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

int LLAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

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
#include "mavenasynparse.h"
#include "mavenitemkeeper.h"
#include "services/project/projectgenerator.h"
#include "services/option/optionmanager.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

class MavenAsynParsePrivate
{
    friend  class MavenAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

MavenAsynParse::MavenAsynParse()
    : d(new MavenAsynParsePrivate)
{

    d->thread = new QThread();
    this->moveToThread(d->thread);

    QObject::connect(this, &Inotify::modified,
                     this, &MavenAsynParse::doDirWatchModify,
                     Qt::DirectConnection);

    QObject::connect(this, &Inotify::createdSub,
                     this, &MavenAsynParse::doWatchCreatedSub,
                     Qt::DirectConnection);

    QObject::connect(this, &Inotify::deletedSub,
                     this, &MavenAsynParse::doWatchDeletedSub,
                     Qt::DirectConnection);
}

MavenAsynParse::~MavenAsynParse()
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

void MavenAsynParse::loadPoms(const dpfservice::ProjectInfo &info)
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

void MavenAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    createRows(info.sourceFolder());
    emit itemsModified({d->rows, true});
}

void MavenAsynParse::parseActions(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    ProjectInfo proInfo = info;
    if (proInfo.isEmpty()) {
        return;
    }

    QFileInfo xmlFileInfo(proInfo.projectFilePath());
    if (!xmlFileInfo.exists())
        return;

    d->xmlDoc = QDomDocument(xmlFileInfo.fileName());
    QFile xmlFile(xmlFileInfo.filePath());
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed, Can't open xml file: "
                    << xmlFileInfo.filePath();
        return;
    }
    if (!d->xmlDoc.setContent(&xmlFile)) {
        qCritical() << "Failed, Can't load to XmlDoc class: "
                    << xmlFileInfo.filePath();
        return;
    }

    QDomNode n = d->xmlDoc.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "project") {
            n = n.firstChild();
        }

        if (e.tagName() == "dependencies") {
            QDomNode depNode = e.firstChild();
            while (!depNode.isNull()) {
                QDomElement depElem = depNode.toElement();
                if (depElem.tagName() == "dependency") {
                    auto depcyNode = depElem.firstChild();
                    while (!depcyNode.isNull()) {
                        auto depcyElem = depcyNode.toElement();
                        qInfo() << "dependencies.dependency."
                                   + depcyElem.tagName()
                                   + ":" + depcyElem.text();
                        depcyNode = depcyNode.nextSibling();
                    }
                }
                depNode = depNode.nextSibling();
            }
        }

        if (e.tagName() == "build")
        {
            QDomNode findNode = e.firstChild();
            while (!findNode.isNull()) {
                qInfo() << findNode.toElement().tagName();
                auto findElem = findNode.toElement();
                if (findElem.tagName() != "plugins") {
                    if (findElem.hasChildNodes()) {
                        findNode = findElem.firstChild();
                        continue;
                    }
                } else {
                    ProjectActionInfos actionInfos;
                    QDomNode pluginsChild = findElem.firstChild();
                    while (!pluginsChild.isNull()) {
                        auto pluginsElem = pluginsChild.toElement();
                        QDomNode pluginChild = pluginsElem.firstChild();
                        if (pluginsElem.tagName() == "plugin") {
                            while (!pluginChild.isNull()) {
                                auto pluginElem = pluginChild.toElement();
                                if ("artifactId" == pluginElem.tagName()) {
                                    ProjectActionInfo actionInfo;
                                    actionInfo.buildProgram = OptionManager::getInstance()->getMavenToolPath();
                                    actionInfo.workingDirectory = xmlFileInfo.filePath();
                                    QString buildArg = pluginElem.text()
                                            .replace("maven-", "")
                                            .replace("-plugin", "");
                                    actionInfo.buildArguments.append(buildArg);
                                    actionInfo.displyText = buildArg;
                                    actionInfo.tooltip = pluginElem.text();
                                    actionInfos.append(actionInfo);
                                }
                                pluginChild = pluginChild.nextSibling();
                            }
                        } // pluginsElem.tagName() == "plugin"
                        pluginsChild = pluginsChild.nextSibling();
                    }
                    if (!actionInfos.isEmpty())
                        emit parsedActions({actionInfos, true});
                } // buildElem.tagName() == "pluginManagement"
                findNode = findNode.nextSibling();
            }
        }// e.tagName() == "build"
        n = n.nextSibling();
    }
}

void MavenAsynParse::doDirWatchModify(const QString &path)
{
    Q_UNUSED(path)
}

void MavenAsynParse::doWatchCreatedSub(const QString &path)
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

void MavenAsynParse::doWatchDeletedSub(const QString &path)
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
                    if (itemIsDir(child))
                        Inotify::removePath(child->toolTip());
                    currPathItem->removeRow(i);
                }
            }
        } else {
            for (int i = 0; i < d->rows.size(); i++) {
                QStandardItem *child = d->rows[i];
                if (itemDisplayName(child) == deleted) {
                    if (itemIsDir(child))
                        Inotify::removePath(child->toolTip());
                    d->rows.removeAt(i);
                }
            }
        }
    }

    emit itemsModified({d->rows, true});
}

bool MavenAsynParse::isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role) const
{
    if (!t1 || !t2)
        return false;
    return t1->data(role) == t2->data(role);
}

void MavenAsynParse::createRows(const QString &path)
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

void MavenAsynParse::removeRows()
{
    d->rootPath.clear();
    for (int i = 0; i < d->rows.size(); i++) {
        removeSelfSubWatch(d->rows[i]);
    }
    d->rows.clear();
    emit itemsModified({d->rows, true});
}

QList<QStandardItem *> MavenAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

int MavenAsynParse::findRowWithDisplay(QList<QStandardItem *> rowList, const QString &fileName)
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

QStandardItem *MavenAsynParse::findItem(const QString &path,
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

void MavenAsynParse::removeSelfSubWatch(QStandardItem *item)
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

int MavenAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

bool MavenAsynParse::itemIsDir(const QStandardItem *item) const
{
    return QFileInfo(item->toolTip()).isDir();
}

bool MavenAsynParse::itemIsFile(const QStandardItem *item) const
{
    return QFileInfo(item->toolTip()).isFile();
}

QString MavenAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

QStringList MavenAsynParse::pathChildFileNames(const QString &path) const
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

QStringList MavenAsynParse::displayNames(const QList<QStandardItem *> items) const
{
    QStringList names;
    for (int i = 0; i < items.size(); i++) {
        names << items[i]->data(Qt::DisplayRole).toString();
    }
    return names;
}

QStringList MavenAsynParse::createdFileNames(const QString &path) const
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

QStringList MavenAsynParse::deletedFileNames(const QString &path) const
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

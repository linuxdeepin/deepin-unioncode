// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenasynparse.h"
#include "services/project/projectgenerator.h"
#include "services/option/optionmanager.h"
#include "services/project/projectservice.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

static QString kSupportArgs[] = {"clean", "compiler:compile", "compiler:testCompile",
                                     "package", "install", "validate", "dependency:tree",
                                     "dependency:analyze", "site:site", "compile", "verify"};

class MavenAsynParsePrivate
{
    friend class MavenAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

MavenAsynParse::MavenAsynParse()
    : d(new MavenAsynParsePrivate)
{

    QObject::connect(this, &QFileSystemWatcher::directoryChanged,
                     this, &MavenAsynParse::doDirectoryChanged);

    d->thread = new QThread();
    this->moveToThread(d->thread);
    d->thread->start();
}

MavenAsynParse::~MavenAsynParse()
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

void MavenAsynParse::loadPoms(const dpfservice::ProjectInfo &info)
{
    QFile docFile(info.workspaceFolder() + QDir::separator() + "pom.xml");

    if (!docFile.exists()) {
        parsedError("Failed, maven pro not exists!: " + docFile.fileName());
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {;
        parsedError(docFile.errorString());
    }

    if (!d->xmlDoc.setContent(&docFile)) {
        docFile.close();
    }
    docFile.close();
}

void MavenAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    createRows(info.workspaceFolder());
    emit itemsModified(d->rows);
}

// TODO(mozart):get from tool or config.
void MavenAsynParse::parseActions(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    ProjectInfo proInfo = info;
    if (proInfo.isEmpty()) {
        return;
    }

    ProjectActionInfos actionInfos;
    auto addAction = [&](QString &arg){
        ProjectMenuActionInfo actionInfo;
        actionInfo.buildProgram = OptionManager::getInstance()->getMavenToolPath();
        actionInfo.workingDirectory = info.workspaceFolder() + QDir::separator() + "pom.xml";
        actionInfo.buildArguments.append(arg);
        actionInfo.displyText = arg;
        actionInfo.tooltip = arg;
        actionInfos.append(actionInfo);
    };

    for (auto arg : kSupportArgs) {
        addAction(arg);
    }

    emit parsedActions(actionInfos);
}

void MavenAsynParse::doDirectoryChanged(const QString &path)
{
    if (!path.startsWith(d->rootPath))
        return;

    d->rows.clear();

    createRows(d->rootPath);

    emit itemsModified(d->rows);
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
            auto newItem = new QStandardItem(dirItera.fileName());
            newItem->setData(dirItera.filePath(), Project::FileIconRole);
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
            auto newItem = new QStandardItem(fileItera.fileName());
            newItem->setData(fileItera.filePath(), Project::FileIconRole);
            newItem->setToolTip(fileItera.filePath());
            if (!item) {
                d->rows.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
}

QList<QStandardItem *> MavenAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
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

int MavenAsynParse::separatorSize() const
{
    return QString(QDir::separator()).size();
}

QString MavenAsynParse::itemDisplayName(const QStandardItem *item) const
{
    if (!item)
        return "";
    return item->data(Qt::DisplayRole).toString();
}

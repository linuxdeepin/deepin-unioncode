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
#include "treemenu.h"
#include "treeproxy.h"
#include "sendevents.h"
#include "common/common.h"

#include <QFileInfo>
#include <QAction>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QAbstractButton>
#include <QFileSystemWatcher>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QStandardPaths>

const QString NEW_FILE {TreeMenu::tr("New File")};
const QString NEW_FOLDER {TreeMenu::tr("New Folder")};
const QString MOVE_TO_TARSH {TreeMenu::tr("Move To Trash")};
const QString DELETE {TreeMenu::tr("Delete")};
const QString BUILD {TreeMenu::tr("Build")};

class TreeMenuPrivate
{
    friend class TreeMenu;
    QJsonDocument globalJsonDocument;
    QJsonDocument cacheJsonDocument;
    void initBuildSupport();
    QString userBuildSupportFilePath();
    void createNewFileAction(QMenu *menu, const QString &path);
    void createNewFolderAction(QMenu *menu, const QString &path);
    void createMoveToTrash(QMenu *menu, const QString &path);
    void createDeleteAction(QMenu *menu, const QString &path);
    void createBuildAction(QMenu *menu, const QString &path);
};

TreeMenu::TreeMenu(QWidget *parent)
    : QMenu(parent)
    , d(new TreeMenuPrivate)
{
    d->initBuildSupport();
}

TreeMenu::~TreeMenu()
{
    if(d)
        delete d;
}

void TreeMenu::createNewFileAction(const QString &path)
{
    d->createNewFileAction(this, path);
}

void TreeMenu::createNewFolderAction(const QString &path)
{
    d->createNewFolderAction(this, path);
}

void TreeMenu::createMoveToTrash(const QString &path)
{
    d->createMoveToTrash(this, path);
}

void TreeMenu::createDeleteAction(const QString &path)
{
    d->createDeleteAction(this, path);
}

void TreeMenu::createBuildAction(const QString &path)
{
    d->createBuildAction(this, path);
}

void TreeMenuPrivate::initBuildSupport() {

    QString globalConfigBuildFile = support_file::Builder::globalPath();

    QFile globalFile(globalConfigBuildFile);
    if (!globalFile.exists()) {
        qCritical() << "Failed, not found global build menu config file";
        ContextDialog::ok(QString("Failed, not found global build menu config file: %0").arg(globalConfigBuildFile));
        abort();
    }

    if (globalFile.open(QFile::ReadOnly)) {
        globalJsonDocument = QJsonDocument::fromJson(globalFile.readAll());
        globalFile.close();
    } else {
        qCritical() << "Failed, can't open global build menu config file";
        abort();
    }

    QString appConfigLocation = CustomPaths::user(CustomPaths::Configures);
    if (!QDir(appConfigLocation).exists()) {
        QDir().mkpath(appConfigLocation); //创建缓存目录
    }

    QString appConfigBuildSupportFile = support_file::Builder::userPath();
    QFileInfo fileInfo(appConfigBuildSupportFile);
    if (!fileInfo.exists()) {
        QFile::copy(globalConfigBuildFile, fileInfo.filePath());
    }

    QFile cachefile(appConfigBuildSupportFile);
    if (!cachefile.permissions().testFlag(QFile::WriteUser)) {
        cachefile.setPermissions(cachefile.permissions() | QFile::WriteUser);
    }

    if (cachefile.open(QFile::OpenModeFlag::ReadOnly)) {
        cacheJsonDocument = QJsonDocument::fromJson(cachefile.readAll());
        qInfo() << "cacheJsonDocument" << cacheJsonDocument;
        cachefile.close();
    } else {
        qCritical() << "Failed, can't open cache build menu config file";
        abort();
    }
}

QString TreeMenuPrivate::userBuildSupportFilePath()
{
    return CustomPaths::user(CustomPaths::Configures);
}

void TreeMenuPrivate::createNewFileAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(NEW_FILE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [=](){
        TreeProxy::instance().createFile(path);
    });
}

void TreeMenuPrivate::createNewFolderAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(NEW_FOLDER, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, newAction, [=](){
        TreeProxy::instance().createFolder(path);
    });
}

void TreeMenuPrivate::createMoveToTrash(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(MOVE_TO_TARSH, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!info.isWritable() || !parentInfo.isWritable() || !ProcessUtil::hasGio()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [=](){
        qInfo() << ProcessUtil::moveToTrash(path);
    });
}

void TreeMenuPrivate::createDeleteAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;

    menu->addSeparator();

    QAction *newAction = new QAction(DELETE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());

    if (!info.isWritable() || !parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [&](){
        if (QFileInfo(path).isDir())
            TreeProxy::instance().deleteFolder(path);
        if (QFileInfo(path).isFile())
            TreeProxy::instance().deleteFile(path);
    });
}

void TreeMenuPrivate::createBuildAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;

    menu->addSeparator();
    QFileInfo info(path);
    if (!info.isFile())
        return;

    QString buildSystem = support_file::Builder::buildSystem(path);
    if (!buildSystem.isEmpty()) {
        QAction *newAction = new QAction(BUILD, menu);
        menu->addAction(newAction);
        QObject::connect(newAction, &QAction::triggered, [=](){
            SendEvents::buildProject(buildSystem,
                                     info.path() + QDir::separator() + "build",
                                     path, {});
        });
    }
}

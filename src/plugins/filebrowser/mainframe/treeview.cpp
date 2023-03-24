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
#include "treeview.h"
#include "transceiver/sendevents.h"

#include "common/common.h"

#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QStack>

const QString NEW_DOCUMENT_NAME = "NewDocument.txt";
const QString NEW_FOLDER_NAME = "NewFolder";
const QString DELETE_MESSAGE_TEXT {QTreeView::tr("The delete operation will be removed from"
                                                 "the disk and will not be recoverable "
                                                 "after this operation.\nDelete anyway?")};

const QString DELETE_WINDOW_TEXT {QTreeView::tr("Delete Warning")};

class TreeViewPrivate
{
    friend class TreeView;
    QFileSystemModel *model {nullptr};
    QMenu* menu {nullptr};
    QStack<QStringList> moveToTrashStack;
    dpfservice::ProjectInfo proInfo;
};

TreeView::TreeView(QWidget *parent)
    : QTreeView (parent)
    , d (new TreeViewPrivate)
{
    d->model = new QFileSystemModel(this);
    d->menu = new QMenu(this);
    setModel(d->model);
    header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    QObject::connect(this, &QTreeView::doubleClicked, this, &TreeView::doDoubleClicked);
}

TreeView::~TreeView()
{
    if (d) {
        delete d;
    }
}

void TreeView::setProjectInfo(const dpfservice::ProjectInfo &proInfo)
{
    d->proInfo = proInfo;
    d->model->setRootPath(proInfo.workspaceFolder());
    auto index = d->model->index(proInfo.workspaceFolder());
    QTreeView::expand(index);
    QTreeView::setRootIndex(index);
    emit rootPathChanged(proInfo.workspaceFolder());
}

void TreeView::selOpen()
{
    QModelIndexList indexs = selectedIndexes();
    QSet<QString> countPaths;
    for (auto index : indexs) {
        countPaths << d->model->filePath(index);
    }

    for (auto path : countPaths) {
        if (QFileInfo(path).isFile())
            editor.openFile(path);
    }
}

void TreeView::selMoveToTrash()
{
    QModelIndexList indexs = selectedIndexes();
    QSet<QString> countPaths;
    for (auto index : indexs) {
        countPaths << d->model->filePath(index);
    }

    QStringList errFilePaths;
    QStringList okFilePaths;
    bool hasError = false;
    for (auto path : countPaths) {
        bool currErr = !FileOperation::doMoveMoveToTrash(path);
        if (currErr){
            errFilePaths << path;
            hasError = true;
        } else {
            okFilePaths << path;
        }
    }

    if (!hasError) {
        d->moveToTrashStack.push(okFilePaths);
    } else {
        QString errMess;
        for (auto errFilePath : errFilePaths) {
            errMess = QTreeView::tr("Error, Can't move to trash: ") + "\n"
                    + errFilePath  + "\n";
        }
        ContextDialog::ok(errMess);
    }
}

void TreeView::selRemove()
{
    QModelIndexList indexs = selectedIndexes();
    QSet<QString> countPaths;
    for (auto index : indexs) {
        countPaths << d->model->filePath(index);
    }

    bool doDeleta = false;
    auto okCallBack = [&](bool checked) {
        Q_UNUSED(checked);
        doDeleta = true;
    };

    QString mess = DELETE_MESSAGE_TEXT + "\n";
    for (auto path : countPaths)  {
        mess += path  + "\n";
    }

    ContextDialog::okCancel(mess,
                            DELETE_WINDOW_TEXT,
                            QMessageBox::Warning,
                            okCallBack,
                            nullptr);

    if (!doDeleta)
        return;

    bool hasError = false;
    QStringList errFilePaths;
    for (auto currPath : countPaths){
        bool currErr = !FileOperation::doRemove(currPath);
        if (currErr){
            errFilePaths << currPath;
            hasError = true;
        }
    }

    if (hasError)  {
        QString errMess;
        for (auto errFilePath : errFilePaths) {
            errMess = QTreeView::tr("Error, Can't move to trash: ") + "\n"
                    + errFilePath  + "\n";
        }
        ContextDialog::ok(errMess);
    }
}

void TreeView::selNewDocument()
{
    QModelIndexList indexs = selectedIndexes();
    bool hasErr = false;
    QString errString;
    if (indexs.size() == 1) {
        QString filePath = d->model->filePath(indexs[0]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            hasErr = !FileOperation::doNewDocument(filePath, NEW_DOCUMENT_NAME);
            if (hasErr)
                errString =  QTreeView::tr("Error: Can't create New Document");
        } else {
            hasErr = true;
            errString =  QTreeView::tr("Error: Create New Document, parent not's dir");
        }
    }

    if (hasErr)
        ContextDialog::ok(errString);
}

void TreeView::selNewFolder()
{
    QModelIndexList indexs = selectedIndexes();
    bool hasErr = false;
    QString errString;
    if (indexs.size() == 1) {
        QString filePath = d->model->filePath(indexs[0]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            hasErr = !FileOperation::doNewFolder(filePath, NEW_FOLDER_NAME);
            if (hasErr)
                errString =  QTreeView::tr("Error: Can't create new folder");
        } else {
            hasErr = true;
            errString =  QTreeView::tr("Error: Create new folder, parent not's dir");
        }
    }

    if (hasErr)
        ContextDialog::ok(errString);
}

void TreeView::recoverFromTrash()
{
    if (!d->moveToTrashStack.isEmpty()) {
        auto filePaths = d->moveToTrashStack.pop();
        for (auto filePath : filePaths) {
            FileOperation::doRecoverFromTrash(filePath);
        }
    }
}

void TreeView::doDoubleClicked(const QModelIndex &index)
{
    QString filePath = d->model->filePath(index);
    if (QFileInfo(filePath).isFile())
        editor.openFile(filePath);
}

void TreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = QTreeView::indexAt(event->pos());
    if (index.isValid()) {
        d->menu = createContextMenu(selectedIndexes());
    } else {
        d->menu = createEmptyMenu();
    }
    d->menu->exec(viewport()->mapToGlobal(event->pos()));
}

QMenu *TreeView::createContextMenu(const QModelIndexList &indexs)
{
    QMenu *menu = new QMenu();
    bool hasDir = false;
    bool selOne = indexs.size() == 0;
    for (auto index: indexs)  {
        if (d->model->isDir(index))
            hasDir = true;
    }

    QAction *openAction = new QAction(QAction::tr("Open"));
    QObject::connect(openAction, &QAction::triggered, this, &TreeView::selOpen);
    menu->addAction(openAction);
    if (hasDir) {
        openAction->setEnabled(false);
    }

    if (selOne || hasDir) {
        QAction *newFolderAction = new QAction(QAction::tr("New Folder"));
        QAction *newDocumentAction = new QAction(QAction::tr("New Document"));

        QObject::connect(newFolderAction, &QAction::triggered, this, &TreeView::selNewFolder);
        QObject::connect(newDocumentAction, &QAction::triggered, this, &TreeView::selNewDocument);
        menu->addSeparator();
        menu->addAction(newFolderAction);
        menu->addAction(newDocumentAction);
    }

    QAction *moveToTrashAction = new QAction(QAction::tr("Move To Trash"));
    QAction *removeAction = new QAction(QAction::tr("Remove"));
    QObject::connect(moveToTrashAction, &QAction::triggered, this, &TreeView::selMoveToTrash);
    QObject::connect(removeAction, &QAction::triggered, this, &TreeView::selRemove);

    menu->addSeparator();
    menu->addAction(moveToTrashAction);
    menu->addAction(removeAction);
    return menu;
}

QMenu *TreeView::createEmptyMenu()
{
    QMenu *menu = new QMenu();
    QAction *recoverFromTrashAction = new QAction(QAction::tr("Recover From Trash"));
    QObject::connect(recoverFromTrashAction, &QAction::triggered,
                     this, &TreeView::recoverFromTrash);
    menu->addAction(recoverFromTrashAction);
    return menu;
}

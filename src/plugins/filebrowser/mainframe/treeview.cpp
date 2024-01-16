// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeview.h"
#include "transceiver/sendevents.h"

#include "common/common.h"
#include <DMenu>
#include <DHeaderView>

#include <QDebug>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QStack>

DWIDGET_USE_NAMESPACE

const QString NEW_DOCUMENT_NAME = "NewDocument.txt";
const QString NEW_FOLDER_NAME = "NewFolder";
const QString DELETE_MESSAGE_TEXT {DTreeView::tr("The delete operation will be removed from"
                                                 "the disk and will not be recoverable "
                                                 "after this operation.\nDelete anyway?")};

const QString DELETE_WINDOW_TEXT {DTreeView::tr("Delete Warning")};

class TreeViewPrivate
{
    friend class TreeView;
    QFileSystemModel *model {nullptr};
    DMenu* menu {nullptr};
    QStack<QStringList> moveToTrashStack;
    dpfservice::ProjectInfo proInfo;
};

TreeView::TreeView(QWidget *parent)
    : DTreeView (parent)
    , d (new TreeViewPrivate)
{
    setLineWidth(0);
    d->model = new QFileSystemModel(this);
    d->menu = new DMenu(this);
    setModel(d->model);
    header()->setSectionResizeMode(DHeaderView::ResizeMode::ResizeToContents);
    setAlternatingRowColors(true);
    QObject::connect(this, &DTreeView::doubleClicked, this, &TreeView::doDoubleClicked);
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
    DTreeView::expand(index);
    DTreeView::setRootIndex(index);
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
            errMess = DTreeView::tr("Error, Can't move to trash: ") + "\n"
                    + errFilePath  + "\n";
        }
        CommonDialog::ok(errMess);
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

    CommonDialog::okCancel(mess,
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
            errMess = DTreeView::tr("Error, Can't move to trash: ") + "\n"
                    + errFilePath  + "\n";
        }
        CommonDialog::ok(errMess);
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
                errString =  DTreeView::tr("Error: Can't create New Document");
        } else {
            hasErr = true;
            errString =  DTreeView::tr("Error: Create New Document, parent not's dir");
        }
    }

    if (hasErr)
        CommonDialog::ok(errString);
}

void TreeView::selNewFolder()
{
    QModelIndexList indexs = selectedIndexes();
    bool hasErr = false;
    QString errString;
    if (indexs.size() > 0) {
        QString filePath = d->model->filePath(indexs[0]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            hasErr = !FileOperation::doNewFolder(filePath, NEW_FOLDER_NAME);
            if (hasErr)
                errString =  DTreeView::tr("Error: Can't create new folder");
        } else {
            hasErr = true;
            errString =  DTreeView::tr("Error: Create new folder, parent not's dir");
        }
    }

    if (hasErr)
        CommonDialog::ok(errString);
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
    QModelIndex index = DTreeView::indexAt(event->pos());
    if (index.isValid()) {
        d->menu = createContextMenu(selectedIndexes());
    } else {
        d->menu = createEmptyMenu();
    }
    d->menu->exec(viewport()->mapToGlobal(event->pos()));
}

DMenu *TreeView::createContextMenu(const QModelIndexList &indexs)
{
    DMenu *menu = new DMenu();
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

DMenu *TreeView::createEmptyMenu()
{
    DMenu *menu = new DMenu();
    QAction *recoverFromTrashAction = new QAction(QAction::tr("Recover From Trash"));
    QObject::connect(recoverFromTrashAction, &QAction::triggered,
                     this, &TreeView::recoverFromTrash);
    menu->addAction(recoverFromTrashAction);
    return menu;
}

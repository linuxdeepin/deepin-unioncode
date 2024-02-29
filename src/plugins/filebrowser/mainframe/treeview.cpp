// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeview.h"
#include "filebrowserdelegate.h"
#include "transceiver/sendevents.h"

#include "common/common.h"
#include <DMenu>
#include <DHeaderView>
#include <DDialog>
#include <DLineEdit>

#include <QDebug>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QStack>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

const QString NEW_DOCUMENT_NAME = "NewDocument.txt";
const QString NEW_FOLDER_NAME = "NewFolder";
const QString DELETE_MESSAGE_TEXT { DTreeView::tr("The delete operation will be removed from"
                                                  "the disk and will not be recoverable "
                                                  "after this operation.\nDelete anyway?") };

const QString DELETE_WINDOW_TEXT { DTreeView::tr("Delete Warning") };

class TreeViewPrivate
{
    friend class TreeView;
    QFileSystemModel *model { nullptr };
    DMenu *menu { nullptr };
    QStack<QStringList> moveToTrashStack;
    dpfservice::ProjectInfo proInfo;
};

TreeView::TreeView(QWidget *parent)
    : DTreeView(parent), d(new TreeViewPrivate)
{
    setLineWidth(0);
    d->model = new QFileSystemModel(this);
    d->menu = new DMenu(this);
    setModel(d->model);
    setItemDelegate(new FileBrowserDelegate(this));
    header()->setSectionResizeMode(DHeaderView::ResizeMode::ResizeToContents);
    setAlternatingRowColors(true);
    setSelectionMode(SingleSelection);
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
            editor.openFile(QString(), path);
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
        if (currErr) {
            errFilePaths << path;
            hasError = true;
        } else {
            okFilePaths << path;
        }
    }

    if (!hasError) {
        d->moveToTrashStack.push(okFilePaths);
    } else {
        QString errMess = tr("Error, Can't move to trash: ") + "\n" + errFilePaths.join('\n');
        CommonDialog::ok(errMess);
    }
}

void TreeView::selRemove()
{
    QModelIndexList indexs = selectedIndexes();
    QStringList countPaths;
    for (auto index : indexs) {
        countPaths << d->model->filePath(index);
    }

    bool doDeleta = false;
    auto okCallBack = [&](bool checked) {
        Q_UNUSED(checked);
        doDeleta = true;
    };

    QString mess = DELETE_MESSAGE_TEXT + "\n" + countPaths.join('\n');
    CommonDialog::okCancel(mess,
                           DELETE_WINDOW_TEXT,
                           QMessageBox::Warning,
                           okCallBack,
                           nullptr);

    if (!doDeleta)
        return;

    bool hasError = false;
    QStringList errFilePaths;
    for (auto currPath : countPaths) {
        bool currErr = !FileOperation::doRemove(currPath);
        if (currErr) {
            errFilePaths << currPath;
            hasError = true;
        }
    }

    if (hasError) {
        QString errMess = tr("Error, Can't move to trash: ") + "\n" + errFilePaths.join('\n');
        CommonDialog::ok(errMess);
    }
}

void TreeView::selRename()
{
    QModelIndexList indexs = selectedIndexes();
    if (indexs.isEmpty())
        return;

    QString filePath = d->model->filePath(indexs[0]);
    QFileInfo fileInfo(filePath);

    auto dialog = new DDialog(this);
    auto inputEdit = new DLineEdit(dialog);

    inputEdit->setPlaceholderText(tr("New Document Name"));
    inputEdit->lineEdit()->setAlignment(Qt::AlignLeft);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("New Document"));

    dialog->addContent(inputEdit);
    dialog->addButton(tr("Ok"), true, DDialog::ButtonRecommend);

    QObject::connect(dialog, &DDialog::buttonClicked, dialog, [=](){
        QString newFileName = inputEdit->text();
          QString newPath = fileInfo.absoluteDir().filePath(newFileName);
          if (fileInfo.isFile()) {
              QFile file(filePath);
              if (file.rename(newPath)) {
                  qDebug() << "File renamed successfully.";
              } else {
                  qDebug() << "Failed to rename file.";
              }
          } else if (fileInfo.isDir()) {
              QDir dir(filePath);
              if (dir.rename(filePath, newPath)) {
                  qDebug() << "Directory renamed successfully.";
              } else {
                  qDebug() << "Failed to rename directory.";
              }
          }
          dialog->accept();
    });

    dialog->exec();
}

void TreeView::selNewDocument()
{
    QModelIndexList indexs = selectedIndexes();
    if (indexs.isEmpty())
        return;

    QString upDirPath = d->model->filePath(indexs[0]);
    QFileInfo upDirInfo(upDirPath);

    QString errString;

    if (upDirInfo.isDir()) {
        bool success = FileOperation::doNewDocument(upDirPath, NEW_DOCUMENT_NAME);
        if (!success)
            errString = tr("Error: Can't create New Document");
    } else {
        errString = tr("Error: Create New Document, parent not's dir");
    }

    if (!errString.isEmpty()) {
        CommonDialog::ok(errString);
    }
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
                errString = tr("Error: Can't create new folder");
        } else {
            hasErr = true;
            errString = tr("Error: Create new folder, parent not's dir");
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
        editor.openFile(QString(), filePath);
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
    if (indexs.isEmpty())
        return nullptr;

    DMenu *menu = new DMenu();

    QString filePath = d->model->filePath(indexs[0]);
    QFileInfo info(filePath);

    QAction *openAction = new QAction(tr("Open"));
    QObject::connect(openAction, &QAction::triggered, this, &TreeView::selOpen);
    menu->addAction(openAction);
    if (info.isDir()) {
        openAction->setEnabled(false);
        QAction *newFolderAction = new QAction(tr("New Folder"));
        connect(newFolderAction, &QAction::triggered, this, &TreeView::selNewFolder);

        QAction *newDocumentAction = new QAction(tr("New Document"));
        connect(newDocumentAction, &QAction::triggered, this, &TreeView::selNewDocument);

        menu->addSeparator();
        menu->addAction(newFolderAction);
        menu->addAction(newDocumentAction);
    }

    QAction *moveToTrashAction = new QAction(tr("Move To Trash"));
    connect(moveToTrashAction, &QAction::triggered, this, &TreeView::selMoveToTrash);

    QAction *removeAction = new QAction(tr("Remove"));
    connect(removeAction, &QAction::triggered, this, &TreeView::selRemove);

    QAction *rename = new QAction(tr("Rename"));
    connect(rename, &QAction::triggered, this, &TreeView::selRename);

    menu->addSeparator();
    menu->addAction(moveToTrashAction);
    menu->addAction(removeAction);
    menu->addAction(rename);

    return menu;
}

DMenu *TreeView::createEmptyMenu()
{
    DMenu *menu = new DMenu();
    QAction *recoverFromTrashAction = new QAction(tr("Recover From Trash"));
    QObject::connect(recoverFromTrashAction, &QAction::triggered,
                     this, &TreeView::recoverFromTrash);
    menu->addAction(recoverFromTrashAction);
    return menu;
}

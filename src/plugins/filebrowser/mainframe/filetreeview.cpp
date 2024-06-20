// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreeview.h"
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
    friend class FileTreeView;
    QFileSystemModel *model { nullptr };
    DMenu *menu { nullptr };
    QStack<QStringList> moveToTrashStack;
    dpfservice::ProjectInfo proInfo;
};

FileTreeView::FileTreeView(QWidget *parent)
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
    QObject::connect(this, &DTreeView::doubleClicked, this, &FileTreeView::doDoubleClicked);
}

FileTreeView::~FileTreeView()
{
    if (d) {
        delete d;
    }
}

void FileTreeView::setProjectInfo(const dpfservice::ProjectInfo &proInfo)
{
    d->proInfo = proInfo;
    d->model->setRootPath(proInfo.workspaceFolder());
    auto index = d->model->index(proInfo.workspaceFolder());
    DTreeView::expand(index);
    DTreeView::setRootIndex(index);
    emit rootPathChanged(proInfo.workspaceFolder());
}

void FileTreeView::selOpen()
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

void FileTreeView::selMoveToTrash()
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

void FileTreeView::selRemove()
{
    QModelIndexList indexs = selectedIndexes();
    QStringList countPaths;
    for (auto index : indexs) {
        countPaths << d->model->filePath(index);
    }
    // Remove duplicates
    countPaths = countPaths.toSet().toList();

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
        QString errMess = tr("Error, Can't delete: ") + "\n" + errFilePaths.join('\n');
        CommonDialog::ok(errMess);
    }
}

void FileTreeView::selRename()
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

void FileTreeView::selNewDocument()
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

void FileTreeView::selNewFolder()
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

void FileTreeView::recoverFromTrash()
{
    if (!d->moveToTrashStack.isEmpty()) {
        auto filePaths = d->moveToTrashStack.pop();
        for (auto filePath : filePaths) {
            FileOperation::doRecoverFromTrash(filePath);
        }
    }
}

void FileTreeView::doDoubleClicked(const QModelIndex &index)
{
    QString filePath = d->model->filePath(index);
    if (QFileInfo(filePath).isFile())
        editor.openFile(QString(), filePath);
}

void FileTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = DTreeView::indexAt(event->pos());
    if (index.isValid()) {
        d->menu = createContextMenu(selectedIndexes());
    } else {
        d->menu = createEmptyMenu();
    }
    d->menu->exec(viewport()->mapToGlobal(event->pos()));
}

DMenu *FileTreeView::createContextMenu(const QModelIndexList &indexs)
{
    if (indexs.isEmpty())
        return nullptr;

    DMenu *menu = new DMenu();

    QString filePath = d->model->filePath(indexs[0]);
    QFileInfo info(filePath);

    QAction *openAction = new QAction(tr("Open"));
    QObject::connect(openAction, &QAction::triggered, this, &FileTreeView::selOpen);
    menu->addAction(openAction);
    if (info.isDir()) {
        openAction->setEnabled(false);
        QAction *newFolderAction = new QAction(tr("New Folder"));
        connect(newFolderAction, &QAction::triggered, this, &FileTreeView::selNewFolder);

        QAction *newDocumentAction = new QAction(tr("New Document"));
        connect(newDocumentAction, &QAction::triggered, this, &FileTreeView::selNewDocument);

        menu->addSeparator();
        menu->addAction(newFolderAction);
        menu->addAction(newDocumentAction);
    }

    QAction *moveToTrashAction = new QAction(tr("Move To Trash"));
    connect(moveToTrashAction, &QAction::triggered, this, &FileTreeView::selMoveToTrash);

    QAction *removeAction = new QAction(tr("Remove"));
    connect(removeAction, &QAction::triggered, this, &FileTreeView::selRemove);

    QAction *rename = new QAction(tr("Rename"));
    connect(rename, &QAction::triggered, this, &FileTreeView::selRename);

    menu->addSeparator();
    menu->addAction(moveToTrashAction);
    menu->addAction(removeAction);
    menu->addAction(rename);

    return menu;
}

DMenu *FileTreeView::createEmptyMenu()
{
    DMenu *menu = new DMenu();
    QAction *recoverFromTrashAction = new QAction(tr("Recover From Trash"));
    QObject::connect(recoverFromTrashAction, &QAction::triggered,
                     this, &FileTreeView::recoverFromTrash);
    menu->addAction(recoverFromTrashAction);
    return menu;
}

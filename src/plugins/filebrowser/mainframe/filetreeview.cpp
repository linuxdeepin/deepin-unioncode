// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreeview.h"
#include "transceiver/sendevents.h"

#include "common/common.h"
#include "base/baseitemdelegate.h"

#include <DMenu>
#include <DHeaderView>
#include <DDialog>
#include <DLineEdit>

#include <QDebug>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QStack>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

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
    setItemDelegate(new BaseItemDelegate(this));
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
    auto index = d->model->index(d->proInfo.workspaceFolder());
    setRootIndex(index);
    sortByColumn(0, Qt::SortOrder::AscendingOrder);
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

    QString message = tr("Delete operation not be recoverable, delete anyway?");

    DDialog dialog;
    dialog.setMessage(message);
    dialog.setWindowTitle(tr("Delete Warining"));
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.insertButton(0, tr("Cancel"));
    dialog.insertButton(1, tr("OK"));
    dialog.exec();

    if (dialog.result() != QDialog::Accepted) {
        return;
    }

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

    QString filePath = d->model->filePath(indexs.first());
    QFileInfo fileInfo(filePath);

    auto dialog = new DDialog(this);
    auto inputEdit = new DLineEdit(dialog);

    inputEdit->setPlaceholderText(tr("New Document Name"));
    inputEdit->lineEdit()->setAlignment(Qt::AlignLeft);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("New Document"));

    dialog->addContent(inputEdit);
    dialog->addButton(tr("Ok"), true, DDialog::ButtonRecommend);

    QObject::connect(dialog, &DDialog::buttonClicked, dialog, [=]() {
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

void FileTreeView::selNewDocument(const QModelIndex &index)
{
    createNew(NewType::File, index);
}

void FileTreeView::selNewFolder(const QModelIndex &index)
{
    createNew(NewType::Folder, index);
}

void FileTreeView::createNew(NewType type, const QModelIndex &index)
{
    DDialog dialog(this);
    DLineEdit inputEdit(&dialog);

    if (type == NewType::File) {
        inputEdit.setPlaceholderText(tr("New File Name"));
        dialog.setWindowTitle(tr("New File"));
    } else {
        inputEdit.setPlaceholderText(tr("New Folder Name"));
        dialog.setWindowTitle(tr("New Folder"));
    }

    inputEdit.lineEdit()->setAlignment(Qt::AlignLeft);

    dialog.addContent(&inputEdit);
    dialog.addButton(tr("Ok"), true, DDialog::ButtonRecommend);

    int code = dialog.exec();
    if (code == 0) {
        auto filePath = d->model->filePath(index);
        QFileInfo info(filePath);
        if (info.isFile())
            filePath = info.absolutePath();

        createNewOperation(filePath, inputEdit.text(), type);
    }
}

void FileTreeView::createNewOperation(const QString &path, const QString &newName, NewType type)
{
    if (!QFile::exists(path) || newName.isEmpty())
        return;

    bool success;
    if (type == NewType::File) {
        success = FileOperation::doNewDocument(path, newName);
    } else {
        success = FileOperation::doNewFolder(path, newName);
    }

    if (!success)
        CommonDialog::ok(tr("Error: Can't create new document or folder, please check whether the name already exists!"));
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
        d->menu = createContextMenu(index);
    } else {
        d->menu = createEmptyMenu();
    }
    d->menu->exec(QCursor::pos());
}

DMenu *FileTreeView::createContextMenu(const QModelIndex &index)
{
    DMenu *menu = new DMenu();

    QAction *openAction = new QAction(tr("Open"));
    QObject::connect(openAction, &QAction::triggered, this, &FileTreeView::selOpen);
    menu->addAction(openAction);
    openAction->setEnabled(false);

    menu->addSeparator();
    createCommonActions(menu, index);

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
    createCommonActions(menu, rootIndex());

    menu->addSeparator();
    QAction *recoverFromTrashAction = new QAction(tr("Recover From Trash"));
    QObject::connect(recoverFromTrashAction, &QAction::triggered,
                     this, &FileTreeView::recoverFromTrash);
    menu->addAction(recoverFromTrashAction);
    return menu;
}

void FileTreeView::createCommonActions(DMenu *menu, const QModelIndex &index)
{
    menu->addAction(tr("New Folder"), this, std::bind(&FileTreeView::selNewFolder, this, index));
    menu->addAction(tr("New Document"), this, std::bind(&FileTreeView::selNewDocument, this, index));
}

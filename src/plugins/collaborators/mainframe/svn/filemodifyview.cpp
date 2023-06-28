// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basetype.h"

#include "common/common.h"

#include "filemodifyview.h"

#include "GitQlientStyles.h"
#include "FileListDelegate.h"
#include "FileContextMenu.h"

#include <QStandardItemModel>
#include <QPushButton>
#include <QHeaderView>

class FileModifyViewPrivate
{
    friend class FileModifyView;
    QStandardItemModel *model{nullptr};
    int fileDataCol = 1;
};

FileModifyView::FileModifyView(QWidget *parent)
    : QTableView (parent)
    , d (new FileModifyViewPrivate)
{
    d->model = new QStandardItemModel();
    setFocusPolicy(Qt::NoFocus);
    d->model->setColumnCount(2);
    setModel(d->model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    setSelectionMode(QAbstractItemView::SingleSelection);//设置选中单个
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
    horizontalHeader()->hide(); // 隐藏列头方法
    verticalHeader()->hide(); // 隐藏行号方法
    setFrameShape(QFrame::NoFrame);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &FileModifyView::customContextMenuRequested, this, &FileModifyView::showContextMenu);
}

RevisionFile FileModifyView::file(int row)
{
    if (row >= 0 && row < d->model->rowCount()) {
        return createFile(row);
    }
    return {};
}

RevisionFiles FileModifyView::files()
{
    RevisionFiles files;
    for (int i = 0; i < d->model->rowCount(); i++) {
        files << file(i);
    }
    return files;
}

void FileModifyView::clean()
{
    d->model->removeRows(0, d->model->rowCount());
}

void FileModifyView::addFile(const RevisionFile &file)
{
    setUpdatesEnabled(false);
    d->model->appendRow(FileModifyView::createRows(file));
    setUpdatesEnabled(true);
}

void FileModifyView::addFiles(const RevisionFiles &files)
{
    setUpdatesEnabled(false);

    for (auto file : files) {
        d->model->appendRow(FileModifyView::createRows(file));
    }

    setUpdatesEnabled(true);
}

void FileModifyView::setFiles(const RevisionFiles &files)
{
    clean();
    addFiles(files);
}

bool FileModifyView::removeFile(const RevisionFile &file)
{
    for (int row = 0; row < d->model->rowCount(); row++) {
        if (createFile(row) == file) {
            auto rowItem = d->model->takeItem(row);
            if (rowItem) {
                delete rowItem;
                return d->model->removeRow(row);
            }
        }
    }
    return false;
}

int FileModifyView::rowCount()
{
    if (!d->model){
        return 0;
    }
    return d->model->rowCount();
}

void FileModifyView::showContextMenu(const QPoint &pos)
{
    const auto index = indexAt(pos);
    if (index.isValid()) {
        RevisionFile file(
                    index.data(FileModifyRole::FilePathRole).toString(),
                    index.data(FileModifyRole::FileIconTypeRole).toString(),
                    index.data(FileModifyRole::RevisionTypeRole).toString()
                    );
        menuRequest(file, viewport()->mapToGlobal(pos));
    }
}

QList<QStandardItem *> FileModifyView::createRows(const RevisionFile &file)
{
    QFileInfo info(file.filePath);
    QList <QStandardItem *> result;
    result << new QStandardItem(file.revisionType);
    auto item = new QStandardItem(file.displayName);
    QIcon icon;
    if (info.isDir()) {
        item->setData(FileModifyRole::FileIconTypeRole, QFileIconProvider::Folder);
        icon = CustomIcons::icon(QFileIconProvider::Folder);
    } else if (info.isFile()) {
        item->setData(FileModifyRole::FileIconTypeRole, QFileIconProvider::File);
        icon = CustomIcons::icon(QFileIconProvider::File);
    }
    item->setData(file.filePath, FileModifyRole::FilePathRole);
    item->setData(file.revisionType, FileModifyRole::RevisionTypeRole);
    item->setIcon(icon);
    item->setToolTip(file.filePath);
    result << item;
    return result;
}

RevisionFile FileModifyView::createFile(int row)
{
    auto item = d->model->item(row, d->fileDataCol);
    if (item)
        return {    item->data(Qt::DisplayRole).toString(),
                    item->data(FileModifyRole::FilePathRole).toString(),
                    item->data(FileModifyRole::RevisionTypeRole).toString()   };
    return {};
}

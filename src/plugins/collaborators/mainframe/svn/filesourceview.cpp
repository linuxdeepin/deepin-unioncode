// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesourceview.h"
#include "filesourcedelegate.h"

#include <QHeaderView>
#include <QFileSystemModel>
#include <QGridLayout>

class FileSourceViewPrivate
{
    friend class FileSourceView;
    QFileSystemModel *model{nullptr};
    FileSourceDelegate *delegate{nullptr};
};

FileSourceView::FileSourceView(QWidget *parent)
    : DTreeView(parent)
    , d (new FileSourceViewPrivate)
{
    d->model = new QFileSystemModel;
    d->delegate = new FileSourceDelegate;
    d->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
    setModel(d->model);
    header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    setItemDelegate(d->delegate);
}

void FileSourceView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
    setRootIndex(d->model->index(filePath));
}

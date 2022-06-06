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
#include "filesourceview.h"
#include "filesourcedelegate.h"

#include <QFileSystemModel>
#include <QGridLayout>

class FileSourceViewPrivate
{
    friend class FileSourceView;
    QFileSystemModel *model{nullptr};
    FileSourceDelegate *delegate{nullptr};
};

FileSourceView::FileSourceView(QWidget *parent)
    : QTreeView(parent)
    , d (new FileSourceViewPrivate)
{
    d->model = new QFileSystemModel;
    d->delegate = new FileSourceDelegate;
    d->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
    setModel(d->model);
    setItemDelegate(d->delegate);
}

void FileSourceView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
    setRootIndex(d->model->index(filePath));
}

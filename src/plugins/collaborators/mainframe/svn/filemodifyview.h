/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include "basetype.h"

#include <QTableView>
#include <QFileIconProvider>

class QStandardItem;
class FileListDelegate; // gitqlient class;
class FileModifyViewPrivate;
class FileModifyView : public QTableView
{
    Q_OBJECT
    FileModifyViewPrivate *const d;
signals:
    void diffChecked(const RevisionFile &file);
    void menuRequest(const RevisionFile &file, const QPoint &global);

public:
    explicit FileModifyView(QWidget *parent = nullptr);
    RevisionFile file(int row);
    void clean();
    void addFile(const RevisionFile &file);
    void addFiles(const RevisionFiles &files);
    void setFiles(const RevisionFiles &files);
    void removeFile(const RevisionFile &file);
    int rowCount();

private:
    void showContextMenu(const QPoint &);
    QList<QStandardItem *> createRows(const RevisionFile &file);
    RevisionFile createFile(int row);
};

#endif // FILELISTWIDGET_H

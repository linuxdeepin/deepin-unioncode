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

#include <QListWidget>
#include <QFileIconProvider>

class FileListDelegate; // gitqlient class;
class FileModifyWidget : public QListWidget
{
    Q_OBJECT

signals:
    void fileDiffChecked(const RevisionFile &file);
    void fileMenuRequest(const RevisionFile &file, const QPoint &global);

public:
    explicit FileModifyWidget(QWidget *parent = nullptr);
    RevisionFile file(int index);
    void addFile(const RevisionFile &file);
    void addFiles(const RevisionFiles &files);
    void setFiles(const RevisionFiles &files);
    void removeFile(const RevisionFile &file);

private:
    void showContextMenu(const QPoint &);
    QListWidgetItem *createItem(const RevisionFile &file);
    RevisionFile createRevi(const QListWidgetItem *item);
};

#endif // FILELISTWIDGET_H

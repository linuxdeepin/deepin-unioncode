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

#include <QListWidget>
#include "RevisionFiles.h"

class FileListDelegate; // gitqlient class;

namespace collaborators {

class FileListWidget : public QListWidget
{
    Q_OBJECT

signals:
   void signalShowFileHistory(const QString &fileName);
   void signalEditFile(const QString &fileName, int line, int column);

public:
    explicit FileListWidget(QWidget *parent = nullptr);
    void insertFiles(const std::optional<RevisionFiles> &files);

private:
    FileListDelegate *mFileDelegate {nullptr};

    void showContextMenu(const QPoint &);
    void addItem(const QString &label, const QColor &clr);
};

} // namespace collaborators

#endif // FILELISTWIDGET_H

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
#ifndef TREEMENU_H
#define TREEMENU_H

#include <QMenu>

class TreeMenuPrivate;
class TreeMenu : public QMenu
{
    Q_OBJECT
    TreeMenuPrivate *const d;
public:
    explicit TreeMenu(QWidget *parent = nullptr);
    virtual ~TreeMenu();
    void createNewFileAction(const QString &path);
    void createNewFolderAction(const QString &path);
    void createMoveToTrash(const QString &path);
    void createDeleteAction(const QString &path);
    void createBuildAction(const QString &path);
};

#endif // TREEMENU_H

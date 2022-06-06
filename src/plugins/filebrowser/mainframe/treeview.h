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
#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
#include "services/project/projectservice.h"

class TreeViewPrivate;
class TreeView : public QTreeView
{
    Q_OBJECT
    TreeViewPrivate *const d;
public:
    explicit TreeView(QWidget *parent = nullptr);
    virtual ~TreeView();
    void setProjectInfo(const dpfservice::ProjectInfo &proInfo);
public slots:
    void selOpen();
    void selMoveToTrash();
    void selRemove();
    void selNewDocument();
    void selNewFolder();
    void recoverFromTrash();
    void doDoubleClicked(const QModelIndex &index);
signals:
    void rootPathChanged(const QString &folder);
protected:
    void contextMenuEvent(QContextMenuEvent *event);
    virtual QMenu *createContextMenu(const QModelIndexList &indexs);
    virtual QMenu *createEmptyMenu();
};

#endif // TREEVIEW_H

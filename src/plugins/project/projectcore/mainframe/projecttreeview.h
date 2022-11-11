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
#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include "services/project/projectservice.h"

#include <QTreeView>

class QStandardItem;
class ProjectTreeViewPrivate;
class ProjectTreeView : public QTreeView
{
    Q_OBJECT
    ProjectTreeViewPrivate *const d;
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);
    virtual ~ProjectTreeView();
    void appendRootItem(QStandardItem *root);
    void removeRootItem(QStandardItem *root);
    void takeRootItem(QStandardItem *root);
    void expandedProjectDepth(const QStandardItem *root, int depth);
    void expandedProjectAll(const QStandardItem *root);
    QList<dpfservice::ProjectInfo> getAllProjectInfo();
Q_SIGNALS:
    void indexMenuRequest(const QModelIndex &index, QContextMenuEvent *event);
    void itemMenuRequest(QStandardItem *item, QContextMenuEvent *event);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    QMenu *childMenu(const QStandardItem *root, const QStandardItem *child);
    QMenu *rootMenu(QStandardItem *root);
private slots:
    void doItemMenuRequest(QStandardItem *item, QContextMenuEvent *event);
    void doDoubleClieked(const QModelIndex &index);
    void doCloseProject(QStandardItem *root);
    void doShowProjectInfo(QStandardItem *root);
    void doActiveProject(QStandardItem *root);
    void actionNewDocument(const QStandardItem *item);
    void actionDeleteDocument(const QStandardItem *item);
    void creatNewDocument(const QStandardItem *item, const QString &fileName);
};

#endif // PROJECTTREEVIEW_H

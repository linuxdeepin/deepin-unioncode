// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

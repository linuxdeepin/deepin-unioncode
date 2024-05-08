// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREEVIEW_H
#define TREEVIEW_H

#include "services/project/projectservice.h"

#include <DTreeView>
#include <DMenu>

enum class NewType {
    File,
    Folder
};

class TreeViewPrivate;
class FileTreeView : public DTK_WIDGET_NAMESPACE::DTreeView
{
    Q_OBJECT
    TreeViewPrivate *const d;

public:
    explicit FileTreeView(QWidget *parent = nullptr);
    virtual ~FileTreeView();
    void setProjectInfo(const dpfservice::ProjectInfo &proInfo);
public slots:
    void selOpen();
    void selMoveToTrash();
    void selRemove();
    void selRename();
    void selNewDocument();
    void selNewFolder();
    void createNew(NewType type);
    void createNewOperation(const QString &newName, NewType type);
    void recoverFromTrash();
    void doDoubleClicked(const QModelIndex &index);
signals:
    void rootPathChanged(const QString &folder);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    virtual DTK_WIDGET_NAMESPACE::DMenu *createContextMenu(const QModelIndexList &indexs);
    virtual DTK_WIDGET_NAMESPACE::DMenu *createEmptyMenu();
};

#endif   // TREEVIEW_H

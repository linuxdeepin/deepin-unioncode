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

public slots:
    void setProjectInfo(const dpfservice::ProjectInfo &proInfo);
    void selOpen();
    void selMoveToTrash();
    void selRemove();
    void selRename();
    void selNewDocument(const QModelIndex &index);
    void selNewFolder(const QModelIndex &index);
    void createNew(NewType type, const QModelIndex &index);
    void createNewOperation(const QString &path, const QString &newName, NewType type);
    void recoverFromTrash();
    void doDoubleClicked(const QModelIndex &index);
signals:
    void rootPathChanged(const QString &folder);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    DTK_WIDGET_NAMESPACE::DMenu *createContextMenu(const QModelIndex &index);
    DTK_WIDGET_NAMESPACE::DMenu *createEmptyMenu();
    void createCommonActions(DTK_WIDGET_NAMESPACE::DMenu *menu, const QModelIndex &index);
};

#endif   // TREEVIEW_H

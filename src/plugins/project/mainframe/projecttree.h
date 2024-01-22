// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include "services/project/projectservice.h"

#include <DTreeView>
#include <DMenu>

DWIDGET_USE_NAMESPACE
class QStandardItem;
class ProjectTreePrivate;
class ProjectTree : public DTreeView
{
    Q_OBJECT
    ProjectTreePrivate *const d;

public:
    explicit ProjectTree(QWidget *parent = nullptr);
    ~ProjectTree() override;
    void activeProjectInfo(const dpfservice::ProjectInfo &info);
    void activeProjectInfo(const QString &kitName, const QString &language,
                           const QString &workspace);
    void appendRootItem(QStandardItem *root);
    void removeRootItem(QStandardItem *root);
    void takeRootItem(QStandardItem *root);
    void expandedProjectDepth(const QStandardItem *root, int depth);
    void expandedProjectAll(const QStandardItem *root);
    QList<dpfservice::ProjectInfo> getAllProjectInfo();
    dpfservice::ProjectInfo getProjectInfo(const QString &kitName, const QString &workspace) const;
    dpfservice::ProjectInfo getActiveProjectInfo() const;
    bool updateProjectInfo(dpfservice::ProjectInfo &projectInfo);
    bool hasProjectInfo(const dpfservice::ProjectInfo &info) const;

Q_SIGNALS:
    void itemMenuRequest(QStandardItem *item, QContextMenuEvent *event);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    DMenu *childMenu(const QStandardItem *root, const QStandardItem *child);
    DMenu *rootMenu(QStandardItem *root);
    void performDrag();

public slots:
    void itemModified(QStandardItem *item, const QList<QStandardItem *> &childs);

private slots:
    void doItemMenuRequest(QStandardItem *item, QContextMenuEvent *event);
    void doDoubleClicked(const QModelIndex &index);
    void doCloseProject(QStandardItem *root);
    void doShowProjectInfo(QStandardItem *root);
    void doActiveProject(QStandardItem *root);
    void actionNewDocument(const QStandardItem *item);
    void actionDeleteDocument(const QStandardItem *item);
    void actionOpenInTerminal(const QStandardItem *item);
    void creatNewDocument(const QStandardItem *item, const QString &fileName);
};

#endif // PROJECTTREE_H

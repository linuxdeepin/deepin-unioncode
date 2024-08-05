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
    void selectProjectFile(const QString &file);
    void setAutoFocusState(bool state);
	bool getAutoFocusState() const;
    void expandItemByFile(const QStringList &filePaths);
    void focusCurrentFile();
    QList<dpfservice::ProjectInfo> getAllProjectInfo();
    dpfservice::ProjectInfo getProjectInfo(const QString &kitName, const QString &workspace) const;
    dpfservice::ProjectInfo getActiveProjectInfo() const;
    QStandardItem *getActiveProjectItem() const;
    bool updateProjectInfo(dpfservice::ProjectInfo &projectInfo);
    bool hasProjectInfo(const dpfservice::ProjectInfo &info) const;

Q_SIGNALS:
    void itemMenuRequest(QStandardItem *item, QContextMenuEvent *event);
    void itemDeleted(QStandardItem *item);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    DMenu *childMenu(const QStandardItem *root, QStandardItem *child);
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
    void actionNewDirectory(const QStandardItem *item);
    void actionNewDocument(const QStandardItem *item);
    void actionRenameDocument(const QStandardItem *item);
    void actionDeleteDocument(QStandardItem *item);
    void actionOpenInTerminal(const QStandardItem *item);
    bool createNewDirectory(const QStandardItem *item, const QString &dirName);
    void creatNewDocument(const QStandardItem *item, const QString &fileName);
    void renameDocument(const QStandardItem *item, const QString &fileName);
};

#endif // PROJECTTREE_H

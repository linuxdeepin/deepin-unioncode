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
#include "projecttreeview.h"
#include "projectinfodialog.h"
#include "projectselectionmodel.h"
#include "projectdelegate.h"
#include "transceiver/sendevents.h"

#include "services/project/projectservice.h"

#include "common/common.h"

#include <QDebug>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QContextMenuEvent>

using namespace dpfservice;

class ProjectTreeViewPrivate
{
    friend class ProjectTreeView;
    QStandardItemModel *itemModel {nullptr};
    ProjectSelectionModel *sectionModel {nullptr};
    ProjectDelegate *delegate {nullptr};
    int itemDepth(const QStandardItem *item)
    {
        int depth = 0;
        const QStandardItem *current = item;
        while (current->parent()) {
            current = current->parent();
            depth ++;
        }
        return depth;
    }
};

ProjectTreeView::ProjectTreeView(QWidget *parent)
    : QTreeView (parent)
    , d(new ProjectTreeViewPrivate)
{
    setEditTriggers(QTreeView::NoEditTriggers);	          //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		  //一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框
    this->header()->hide();

    d->itemModel = new QStandardItemModel(this);
    setModel(d->itemModel);

    // 右键菜单创建
    QObject::connect(this, &ProjectTreeView::itemMenuRequest,
                     this, &ProjectTreeView::doItemMenuRequest);


    // 双击操作
    QObject::connect(this, &ProjectTreeView::doubleClicked,
                     this, &ProjectTreeView::doDoubleClieked);

    d->sectionModel = new ProjectSelectionModel(d->itemModel);
    setSelectionModel(d->sectionModel);

    d->delegate = new ProjectDelegate(this);
    setItemDelegate(d->delegate);
}

ProjectTreeView::~ProjectTreeView()
{
    if (d) {
        delete d;
    }
}

void ProjectTreeView::appendRootItem(QStandardItem *root)
{
    if (!root)
        return;

    // 发送工程创建信号
    using namespace dpfservice;
    auto info = ProjectInfo::get(ProjectGenerator::root(root));

    // 添加工程节点
    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    if (model)
        model->appendRow(root);

    // 发送工程节点已创建信号
    SendEvents::projectCreated(info);

    // 激活当前工程节点
    doActiveProject(root);
}

void ProjectTreeView::removeRootItem(QStandardItem *root)
{
    this->viewport()->setUpdatesEnabled(false);

    ProjectInfo info = ProjectInfo::get(ProjectGenerator::root(root));

    this->takeRootItem(root);

    // 从生成器中删除
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    auto generator = projectService->createGenerator<ProjectGenerator>(info.kitName());
    if (generator)
        generator->removeRootItem(root);

    // 发送工程删除信号
    SendEvents::projectDeleted(info);

    // 始终保持首选项
    int rowCount = d->itemModel->rowCount();
    if ( 0 < rowCount) { // 存在其他工程时
        auto index = d->itemModel->index(0, 0);
        doActiveProject(d->itemModel->itemFromIndex(index));
    }

    this->viewport()->setUpdatesEnabled(true);
}

void ProjectTreeView::takeRootItem(QStandardItem *root)
{
    // 从展示的模型中删除
    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    QModelIndex index = model->indexFromItem(root);
    model->takeRow(index.row());
}

void ProjectTreeView::doItemMenuRequest(QStandardItem *item, QContextMenuEvent *event)
{
    auto rootItem = ProjectGenerator::root(item);
    QMenu *menu = nullptr;

    if (rootItem == item) {
        menu = rootMenu(rootItem);
    } else {
        menu = childMenu(rootItem, item);
    }

    if (menu) {
        menu->move(event->globalPos());
        menu->exec();
        delete menu;
    }
}

void ProjectTreeView::expandedProjectDepth(const QStandardItem *root, int depth)
{
    if (!root)
        return;

    qInfo() << root->data(Qt::DisplayRole);
    if (d->itemDepth(root) < depth) { //满足深度
        expand(d->itemModel->indexFromItem(root));
        for(int i = 0; i < root->rowCount(); i++) {
            QStandardItem * childitem = root->child(i);
            if (root->hasChildren()) {
                expandedProjectDepth(childitem, depth);
            }
        }
    }
}

void ProjectTreeView::expandedProjectAll(const QStandardItem *root)
{
    if (!root)
        return;

    expand(d->itemModel->indexFromItem(root));
    if (root->hasChildren()) {
        for(int i = 0; i < root->rowCount(); i++) {
            QStandardItem * childitem = root->child(i);
            expandedProjectAll(childitem);
        }
    }
}

QList<dpfservice::ProjectInfo> ProjectTreeView::getAllProjectInfo()
{
    using namespace dpfservice;
    QList<ProjectInfo> result;
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        result <<  ProjectInfo::get(d->itemModel->index(row, 0));
    }
    return result;
}

void ProjectTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeView::contextMenuEvent(event);
    QModelIndex index = indexAt(event->pos());
    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    indexMenuRequest(index, event);
    itemMenuRequest(d->itemModel->itemFromIndex(index), event);
}

QMenu *ProjectTreeView::childMenu(const QStandardItem *root, const QStandardItem *child)
{
    QMenu *menu = nullptr;
    QString toolKitName = ProjectInfo::get(root).kitName();
    // 获取支持右键菜单生成器
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        menu = projectService->createGenerator<ProjectGenerator>(toolKitName)->createItemMenu(child);
    }
    return menu;
}

QMenu *ProjectTreeView::rootMenu(QStandardItem *root)
{
    QMenu * menu = nullptr;
    QString toolKitName = ProjectInfo::get(root).kitName();
    // 获取支持右键菜单生成器
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        menu = projectService->createGenerator<ProjectGenerator>(toolKitName)->createItemMenu(root);
    }
    if (!menu)
        menu = new QMenu();

    QAction* activeProjectAction = new QAction(QAction::tr("Project Active"));
    QAction* closeAction = new QAction(QAction::tr("Project Close"));
    QAction* propertyAction = new QAction(QAction::tr("Project Info"));
    QObject::connect(activeProjectAction, &QAction::triggered, [=](){doActiveProject(root);});
    QObject::connect(closeAction, &QAction::triggered, [=](){doCloseProject(root);});
    QObject::connect(propertyAction, &QAction::triggered, [=](){doShowProjectInfo(root);});
    menu->insertAction(nullptr, activeProjectAction);
    menu->insertAction(nullptr, closeAction);
    menu->insertAction(nullptr, propertyAction);
    return menu;
}

void ProjectTreeView::doDoubleClieked(const QModelIndex &index)
{
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    if (info.exists() && info.isFile()) {
        QString workspaceFolder, language;
        QModelIndex rootIndex = ProjectGenerator::root(index);
        if (rootIndex.isValid()) {
            auto info = ProjectInfo::get(rootIndex);
            workspaceFolder = info.workspaceFolder();
            language = info.language();
        }
        if (!workspaceFolder.isEmpty() && !language.isEmpty()) {
            SendEvents::doubleCliekedOpenFile(workspaceFolder, language, info.filePath());
        } else {
            ContextDialog::ok(QDialog::tr("Can't find workspace from file :%0").arg(info.filePath()));
        }
    }
}

void ProjectTreeView::doCloseProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;
    auto info = ProjectInfo::get(root);
    this->removeRootItem(root);
}

void ProjectTreeView::doActiveProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;
    d->delegate->setActiveProject(d->itemModel->indexFromItem(root));
    SendEvents::projectActived(ProjectInfo::get(root));
}

void ProjectTreeView::doShowProjectInfo(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;

    ProjectInfoDialog dialog;
    QString propertyText = "Language: " + ProjectInfo::get(root).language() + "\n"
            + "KitName: " + ProjectInfo::get(root).kitName() + "\n"
            + "BuildFolder: " + ProjectInfo::get(root).buildFolder() + "\n"
            + "SourceFolder: " + ProjectInfo::get(root).sourceFolder() + "\n"
            + "WorkspaceFolder: " + ProjectInfo::get(root).workspaceFolder() + "\n"
            + "ProjectFilePath: " + ProjectInfo::get(root).projectFilePath() + "\n"
            + "BuildType: " + ProjectInfo::get(root).buildType() + "\n"
            + "BuildCustomArgs: " + "\n    " + ProjectInfo::get(root).buildCustomArgs().join("\n    ");
    dialog.setPropertyText(propertyText);
    dialog.exec();
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projecttree.h"
#include "projectinfodialog.h"
#include "projectselectionmodel.h"
#include "projectdelegate.h"
#include "projectmodel.h"
#include "transceiver/sendevents.h"

#include "services/project/projectservice.h"

#include "common/common.h"

#include <QDebug>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QDrag>
#include <QApplication>
#include <QUrl>

const QString DELETE_MESSAGE_TEXT {QTreeView::tr("The delete operation will be removed from"
                                                 "the disk and will not be recoverable "
                                                 "after this operation.\nDelete anyway?")};

using namespace dpfservice;

class ProjectTreePrivate
{
    friend class ProjectTree;
    ProjectModel *itemModel {nullptr};
    ProjectSelectionModel *sectionModel {nullptr};
    ProjectDelegate *delegate {nullptr};
    QPoint startPos;
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

ProjectTree::ProjectTree(QWidget *parent)
    : QTreeView (parent)
    , d(new ProjectTreePrivate)
{
    setEditTriggers(QTreeView::NoEditTriggers);	          //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		  //一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框
    this->header()->hide();

    d->itemModel = new ProjectModel(this);
    setModel(d->itemModel);

    // 右键菜单创建
    QObject::connect(this, &ProjectTree::itemMenuRequest,
                     this, &ProjectTree::doItemMenuRequest);


    // 双击操作
    QObject::connect(this, &ProjectTree::doubleClicked,
                     this, &ProjectTree::doDoubleClieked);

    d->sectionModel = new ProjectSelectionModel(d->itemModel);
    setSelectionModel(d->sectionModel);

    d->delegate = new ProjectDelegate(this);
    setItemDelegate(d->delegate);
    this->setDragEnabled(true);
}

ProjectTree::~ProjectTree()
{
    if (d) {
        delete d;
    }
}

void ProjectTree::activeProjectInfo(const ProjectInfo &info)
{
    int rowCount = d->itemModel->rowCount();
    for (int currRow = 0; currRow < rowCount; currRow ++) {
        auto currItem = d->itemModel->item(currRow, 0);
        if (currItem) {
            auto currInfo = ProjectInfo::get(ProjectGenerator::root(currItem));
            if (currInfo.language() == info.language()
                    && currInfo.workspaceFolder() == info.workspaceFolder()
                    && currInfo.kitName() == info.kitName()) {
                doActiveProject(currItem);
            }
        }
    }
}

void ProjectTree::activeProjectInfo(const QString &kitName,
                                    const QString &language,
                                    const QString &workspace)
{
    int rowCount = d->itemModel->rowCount();
    for (int currRow = 0; currRow < rowCount; currRow ++) {
        auto currItem = d->itemModel->item(currRow, 0);
        if (currItem) {
            auto currInfo = ProjectInfo::get(ProjectGenerator::root(currItem));
            if (currInfo.language() == language
                    && currInfo.workspaceFolder() == workspace
                    && currInfo.kitName() == kitName) {
                doActiveProject(currItem);
            }
        }
    }
}

void ProjectTree::appendRootItem(QStandardItem *root)
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

void ProjectTree::removeRootItem(QStandardItem *root)
{
    this->viewport()->setUpdatesEnabled(false);

    ProjectInfo info = ProjectInfo::get(ProjectGenerator::root(root));

    // 从Model中移除
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

void ProjectTree::takeRootItem(QStandardItem *root)
{
    // 从展示的模型中删除
    QModelIndex index = d->itemModel->indexFromItem(root);
    d->itemModel->takeRow(index.row());
}

void ProjectTree::doItemMenuRequest(QStandardItem *item, QContextMenuEvent *event)
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

void ProjectTree::expandedProjectDepth(const QStandardItem *root, int depth)
{
    if (!root)
        return;

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

void ProjectTree::expandedProjectAll(const QStandardItem *root)
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

QList<dpfservice::ProjectInfo> ProjectTree::getAllProjectInfo()
{
    using namespace dpfservice;
    QList<ProjectInfo> result;
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        result <<  ProjectInfo::get(d->itemModel->index(row, 0));
    }
    return result;
}

ProjectInfo ProjectTree::getProjectInfo(const QString &kitName, const QString &workspace)
{
    ProjectInfo projectInfo;
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        ProjectInfo info = ProjectInfo::get(d->itemModel->index(row, 0));
        if (kitName == info.kitName() && workspace == info.workspaceFolder()) {
            projectInfo = info;
            break;
        }
    }
    return projectInfo;
}

ProjectInfo ProjectTree::getActiveProjectInfo() const
{
    ProjectInfo projectInfo;
    auto activeProject = d->delegate->getActiveProject();
    if(activeProject.isValid()) {
        projectInfo = ProjectInfo::get(activeProject);
    }
    return projectInfo;
}

void ProjectTree::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeView::contextMenuEvent(event);
    QModelIndex index = indexAt(event->pos());
    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    indexMenuRequest(index, event);
    itemMenuRequest(d->itemModel->itemFromIndex(index), event);
}

void ProjectTree::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        d->startPos = event->pos();
    QTreeView::mousePressEvent(event);
}

void ProjectTree::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - d->startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    QTreeView::mouseMoveEvent(event);
}

QMenu *ProjectTree::childMenu(const QStandardItem *root, const QStandardItem *child)
{
    QMenu *menu = nullptr;
    QString toolKitName = ProjectInfo::get(root).kitName();
    // 获取支持右键菜单生成器
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        menu = projectService->createGenerator<ProjectGenerator>(toolKitName)->createItemMenu(child);
    }
    if (!menu)
        menu = new QMenu();


    QAction *newDocAction = new QAction(tr("New Document"));
    menu->addAction(newDocAction);
    QObject::connect(newDocAction, &QAction::triggered, this, [=](){
        actionNewDocument(child);
    });

    bool isDir = false;
    QModelIndex index = d->itemModel->indexFromItem(child);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    if (info.isDir()) {
        isDir = true;
    }

    QAction *deleteDocAction = new QAction(tr("Delete Document"));
    menu->addAction(deleteDocAction);
    QObject::connect(deleteDocAction, &QAction::triggered, this, [=](){
        actionDeleteDocument(child);
    });
    if (isDir) {
        deleteDocAction->setEnabled(false);
    }
    return menu;
}

QMenu *ProjectTree::rootMenu(QStandardItem *root)
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

    QAction* activeProjectAction = new QAction(QAction::tr("Project Active"), menu);
    QAction* closeAction = new QAction(QAction::tr("Project Close"), menu);
    QAction* propertyAction = new QAction(QAction::tr("Project Info"), menu);
    QObject::connect(activeProjectAction, &QAction::triggered, activeProjectAction, [=](){doActiveProject(root);});
    QObject::connect(closeAction, &QAction::triggered, closeAction, [=](){doCloseProject(root);});
    QObject::connect(propertyAction, &QAction::triggered, propertyAction, [=](){doShowProjectInfo(root);});
    menu->insertAction(nullptr, activeProjectAction);
    menu->insertAction(nullptr, closeAction);
    menu->insertAction(nullptr, propertyAction);
    return menu;
}

void ProjectTree::performDrag()
{
    QModelIndex index = currentIndex();
    QStandardItem *item = d->itemModel->itemFromIndex(index);
    if (item) {
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        QString filePath = "file:" + index.data(Qt::ToolTipRole).toString();
        urls.append(QUrl(filePath));
        mimeData->setUrls(urls);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}

void ProjectTree::itemModified(QStandardItem *item, const QList<QStandardItem *> &childs)
{
    setUpdatesEnabled(false);
    auto parentIndex = d->itemModel->indexFromItem(item);
    int childCount = d->itemModel->rowCount(parentIndex);
    d->itemModel->removeRows(0, childCount, parentIndex);
    item->appendRows(childs);
    setUpdatesEnabled(true);
}

void ProjectTree::doDoubleClieked(const QModelIndex &index)
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
        editor.openFileWithKey(workspaceFolder, language, info.filePath());
    }
}

void ProjectTree::doCloseProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;
    auto info = ProjectInfo::get(root);
    this->removeRootItem(root);
}

void ProjectTree::doActiveProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;
    d->delegate->setActiveProject(d->itemModel->indexFromItem(root));
    SendEvents::projectActived(ProjectInfo::get(root));
}


void ProjectTree::actionNewDocument(const QStandardItem *item)
{
    QDialog *dlg = new QDialog;
    QLineEdit *edit = new QLineEdit;

    edit->setAlignment(Qt::AlignLeft);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("New Document"));
    dlg->resize(400, 100);

    QVBoxLayout *vLayout = new QVBoxLayout(dlg);
    QPushButton *pbtOk = new QPushButton(tr("ok"));
    pbtOk->setFixedSize(40, 20);
    vLayout->addWidget(edit);
    vLayout->addWidget(pbtOk, 0, Qt::AlignCenter);

    QObject::connect(pbtOk, &QPushButton::clicked, dlg, [=](){
        creatNewDocument(item, edit->text());
        dlg->close();
    });

    dlg->exec();
}

void ProjectTree::actionDeleteDocument(const QStandardItem *item)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    if (!info.isFile())
        return;

    bool doDelete = false;
    auto okCallBack = [&](bool checked) {
        Q_UNUSED(checked);
        doDelete = true;
    };

    QString mess = DELETE_MESSAGE_TEXT + "\n" + info.filePath();
    ContextDialog::okCancel(mess,
                            DELETE_MESSAGE_TEXT,
                            QMessageBox::Warning,
                            okCallBack,
                            nullptr);

    if (!doDelete)
        return;
    QFile(info.filePath()).remove();
}

void ProjectTree::creatNewDocument(const QStandardItem *item, const QString &fileName)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    QString workspace, language;
    QModelIndex rootIndex = ProjectGenerator::root(index);
    if (rootIndex.isValid()) {
        auto rootInfo = ProjectInfo::get(rootIndex);
        workspace = rootInfo.workspaceFolder();
        language = rootInfo.language();
    }

    QString filePath;
    if (info.isDir()) {
        filePath = info.filePath() + QDir::separator() + fileName;
    } else if (info.isFile()) {
        filePath = info.path() + QDir::separator() + fileName;
    }

    if (QFile::exists(filePath)) {
        bool doOverWrite = false;
        auto okCallBack = [&](bool checked) {
            Q_UNUSED(checked);
            doOverWrite = true;
        };

        QString mess = "A file with name " + fileName + " already exists. Would you like to overwrite it?";
        ContextDialog::okCancel(mess,
                                DELETE_MESSAGE_TEXT,
                                QMessageBox::Warning,
                                okCallBack,
                                nullptr);
        if (doOverWrite) {
            QFile::remove(filePath);
        }
    }

    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::NewOnly)) {
        file.close();
    }
    editor.openFileWithKey(workspace, language, filePath);
}

void ProjectTree::doShowProjectInfo(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;

    ProjectInfoDialog dialog;
    QString propertyText = "Language: " + ProjectInfo::get(root).language() + "\n"
            + "KitName: " + ProjectInfo::get(root).kitName() + "\n"
            + "BuildFolder: " + ProjectInfo::get(root).buildFolder() + "\n"
            + "WorkspaceFolder: " + ProjectInfo::get(root).workspaceFolder() + "\n"
            + "BuildType: " + ProjectInfo::get(root).buildType() + "\n"
            + "BuildProgram: " + "\n    " + ProjectInfo::get(root).buildProgram() + "\n"
            + "ConfigCustomArgs: " + "\n    " + ProjectInfo::get(root).configCustomArgs().join(" ") + "\n"
            + "BuildCustomArgs: " + "\n    " + ProjectInfo::get(root).buildCustomArgs().join(" ") + "\n"
            + "CleanCustomArgs: " + "\n    " + ProjectInfo::get(root).cleanCustomArgs().join(" ");
    dialog.setPropertyText(propertyText);
    dialog.exec();
}

#include "projecttreeview.h"
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
    QStandardItemModel *model{nullptr};
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

    d->model = new QStandardItemModel(this);
    setModel(d->model);

    // 右键菜单创建
    QObject::connect(this, &ProjectTreeView::itemMenuRequest,
                     this, &ProjectTreeView::doItemMenuRequest);


    // 双击操作
    QObject::connect(this, &ProjectTreeView::doubleClicked,
                     this, &ProjectTreeView::doDoubleClieked);
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
    QString workspaceFolder = info.workspaceFolder();
    QString language = info.language();
    QString buildFolder = info.buildFolder();
    if (!workspaceFolder.isEmpty() && !language.isEmpty()) {
        lsp::ClientManager::instance()->initClient({workspaceFolder, language}, buildFolder);
        SendEvents::projectCreate(workspaceFolder, language, buildFolder);
    }

    // 添加工程节点
    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    if (model)
        model->appendRow(root);
}

void ProjectTreeView::removeRootItem(const QStandardItem *root)
{
    using namespace dpfservice;
    auto info = ProjectInfo::get(ProjectGenerator::root(root));
    QString workspaceFolder = info.workspaceFolder();
    QString language = info.language();

    // 遍历删除工程根节点
    bool isDeleted = false;
    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    if (model) {
        for (int row = 0; row < model->rowCount(); row ++) {
            if (info == ProjectInfo::get(model->item(row))) {
                model->removeRow(row);
                isDeleted = true;
            }
        }
    }

    // 发送工程删除信号
    if (!workspaceFolder.isEmpty() && !language.isEmpty() && isDeleted) {
        lsp::ClientManager::instance()->shutdownClient({workspaceFolder, language});
        SendEvents::projectDelete(workspaceFolder, language);
    }
}

void ProjectTreeView::doItemMenuRequest(const QStandardItem *item, QContextMenuEvent *event)
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    auto rootItem = ProjectGenerator::root(item);
    QString toolKitName = ProjectInfo::get(rootItem).kitName();
    // 获取支持右键菜单生成器
    if (projectService->supportGeneratorName().contains(toolKitName)) {
        QMenu *itemMenu = projectService->createGenerator(toolKitName)->createItemMenu(item);
        if (itemMenu) {
            itemMenu->move(event->globalPos());
            itemMenu->exec();
            delete itemMenu;
        } else if (rootItem == item) {
            QMenu * rootItemMenu = new QMenu;
            QAction* closeAction = new QAction(QAction::tr("Close"));
            QAction* propertyAction = new QAction(QAction::tr("Property"));
            QObject::connect(closeAction, &QAction::triggered, [=](){
                this->removeRootItem(rootItem);
            });
            QObject::connect(propertyAction, &QAction::triggered, [=](){
                qInfo() << "project root item show property";
            });
            rootItemMenu->addAction(closeAction);
            rootItemMenu->addAction(propertyAction);
            rootItemMenu->move(event->globalPos());
            rootItemMenu->exec();
            delete rootItemMenu;
        } else {

        }
    }
};

void ProjectTreeView::expandedProjectDepth(const QStandardItem *root, int depth)
{
    if (!root)
        return;

    qInfo() << root->data(Qt::DisplayRole);
    if (d->itemDepth(root) < depth) { //满足深度
        expand(d->model->indexFromItem(root));
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

    expand(d->model->indexFromItem(root));
    if (root->hasChildren()) {
        for(int i = 0; i < root->rowCount(); i++) {
            QStandardItem * childitem = root->child(i);
            expandedProjectAll(childitem);
        }
    }
}

void ProjectTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeView::contextMenuEvent(event);
    QModelIndex index = indexAt(event->pos());
    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    indexMenuRequest(index, event);
    itemMenuRequest(d->model->itemFromIndex(index), event);
}

void ProjectTreeView::doDoubleClieked(const QModelIndex &index) {
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

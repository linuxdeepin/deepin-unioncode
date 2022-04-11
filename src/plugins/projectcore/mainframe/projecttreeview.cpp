#include "projecttreeview.h"

#include <QDebug>
#include <QStandardItemModel>
class ProjectTreeViewPrivate
{
    friend class ProjectTreeView;
    QStandardItemModel *model{nullptr};

    int itemDepth(QStandardItem *item)
    {
        int depth = 0;
        QStandardItem *current = item;
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
    setEditTriggers(QTreeView::NoEditTriggers);	        //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		//一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setAlternatingRowColors(true);                        //每间隔一行颜色不一样
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框

    d->model = new QStandardItemModel(this);
    setModel(d->model);
}

ProjectTreeView::~ProjectTreeView()
{
    if (d) {
        delete d;
    }
}

void ProjectTreeView::appendRootItem(QStandardItem *item)
{
    if (!item)
        return;

    qInfo() << __FUNCTION__ << "item name" << item->text();

    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    model->appendRow(item);
}

void ProjectTreeView::expandedProjectDepth(QStandardItem *rootItem, int depth)
{
    Q_ASSERT(rootItem);
    qInfo() << rootItem->data(Qt::DisplayRole);
    if (d->itemDepth(rootItem) < depth) { //满足深度
        expand(d->model->indexFromItem(rootItem));
        for(int i = 0; i < rootItem->rowCount() ;i++) {
            QStandardItem * childitem = rootItem->child(i);
            if (rootItem->hasChildren()) {
                expandedProjectDepth(childitem, depth);
            }
        }
    }
}

void ProjectTreeView::expandedProjectAll(QStandardItem *rootItem)
{
    expand(d->model->indexFromItem(rootItem));
    if (rootItem->hasChildren()) {
        for(int i = 0;i < rootItem->rowCount() ;i++) {
            QStandardItem * childitem = rootItem->child(i);
            expandedProjectAll(childitem);
        }
    }
}

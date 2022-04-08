#include "projecttreeview.h"

#include <QDebug>
#include <QStandardItemModel>

ProjectTreeView::ProjectTreeView(QWidget *parent)
    : QTreeView (parent)
{
    setEditTriggers(QTreeView::NoEditTriggers);	        //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		//一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setAlternatingRowColors(true);                        //每间隔一行颜色不一样
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框

    auto model = new QStandardItemModel(this);
    setModel(model);
}

void ProjectTreeView::appendRootItem(QStandardItem *item)
{
    if (!item)
        return;

    qInfo() << __FUNCTION__ << "item name" << item->text();

    QStandardItemModel *model = static_cast<QStandardItemModel*>(QTreeView::model());
    model->appendRow(item);
}

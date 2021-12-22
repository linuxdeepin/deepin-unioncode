#include "treeview.h"
#include "treemodel.h"
#include "treeproxy.h"
#include "treemenu.h"
#include "sendevents.h"
#include "treeviewdelegate.h"

#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

class TreeViewPrivate
{
    friend class TreeView;
    TreeModel *model;
    TreeViewDelegate *delegate;
    const QString RENAME { TreeView::tr("Rename") };
    const QString CLOSE_ALL { TreeView::tr("Close All") };
    const QString CLOSE { TreeView::tr("Close") };
};

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
    , d(new TreeViewPrivate())
{

    d->model = new TreeModel(this);
    setModel(d->model);

    d->delegate = new TreeViewDelegate(this);
    setItemDelegate(d->delegate);

    setHeaderHidden(true);

    setEditTriggers(EditKeyPressed);

    QObject::connect(this, &QTreeView::doubleClicked,
                     this, &TreeView::doubleClicked);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::appendedFile,
                     this, &TreeView::selectNode);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::appendedFolder,
                     this, &TreeView::selectNode);

    QObject::connect(d->model, &TreeModel::renamedChild,
                     this, &TreeView::selectNode, Qt::QueuedConnection);

    QObject::connect(d->model, &TreeModel::createdChild,
                     this, &TreeView::selectNode, Qt::QueuedConnection);

    QObject::connect(d->model, &TreeModel::createdChild,
                     this, &TreeView::rename, Qt::QueuedConnection);
}

TreeView::~TreeView()
{

}

void TreeView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid() && !isExpanded(index)) {
        Node* pressNode = static_cast<Node*>(index.internalPointer());
        if (pressNode) {
            TreeProxy::instance().loadChildren(pressNode);
            pressNode->isLoaded = true; //初始化数据
        }
    }
    return QTreeView::mousePressEvent(event);
}

void TreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        Node *filesNode = TreeProxy::instance().files();
        Node *foldersNode = TreeProxy::instance().folders();
        TreeMenu menu(this);
        menu.move(this->mapToGlobal(event->pos()));

        if (node == filesNode || node == foldersNode) {
            QAction *colseAllAction = new QAction(d->CLOSE_ALL, &menu);
            QAction::connect(colseAllAction, &QAction::triggered, this, [=](){
                this->closeAll(node); // close all node
            });
            menu.addAction(colseAllAction);
        }

        if (node->parent == filesNode || node->parent == foldersNode) {
            QAction *colseAllAction = new QAction(d->CLOSE, &menu);
            QAction::connect(colseAllAction, &QAction::triggered, colseAllAction, [=](){
                if (node->parent == filesNode)
                    this->closeTheFile(node); // close the file
                if (node->parent == foldersNode)
                    this->closeTheFolder(node); // colse the folder
            });
            menu.addAction(colseAllAction);
        }

        if (node->parent && node->parent != foldersNode
                && node->parent != filesNode) {
            QAction *renameAction = new QAction(d->RENAME, &menu);
            QAction::connect(renameAction, &QAction::triggered, renameAction, [=](){
                this->rename(node);
            });
            menu.addAction(renameAction);
            menu.createNewFileAction(node->toolTip);
            menu.createNewFolderAction(node->toolTip);
            menu.createMoveToTrash(node->toolTip);
            menu.createBuildAction(node->toolTip);
            menu.createDeleteAction(node->toolTip);
        }

        menu.exec();
    }
}

void TreeView::selectNode(Node *node)
{
    if (!node)
        return;

    selectionModel()->clear();

    QModelIndex expandIndex = d->model->index(TreeProxy::instance().root(node->parent));
    expand(expandIndex);

    QModelIndex selectIndex = d->model->index(node);
    selectionModel()->select(selectIndex, QItemSelectionModel::ClearAndSelect);
}

void TreeView::closeTheFile(Node *node)
{
    if (d && d->model)
        d->model->removeFile(node);
}

void TreeView::closeTheFolder(Node *node)
{
    if (d && d->model)
        d->model->removeFolder(node);
}

void TreeView::closeAll(Node *node)
{
    Node *filesNode = TreeProxy::instance().files();
    Node *foldersNode = TreeProxy::instance().folders();
    if (node == filesNode)
        d->model->removeAllFile();
    if (node == foldersNode)
        d->model->removeAllFolder();
}

void TreeView::rename(Node *node)
{
    auto renameIndex = d->model->index(node);

    if (!node && !node->parent)
        return;

    if (node->parent == TreeProxy::instance().files()
            || node->parent == TreeProxy::instance().folders()
            || node == TreeProxy::instance().files()
            || node == TreeProxy::instance().folders())
        return;

    emit edit(renameIndex);
}

void TreeView::doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        Node *filesNode = TreeProxy::instance().files();
        Node *foldersNode = TreeProxy::instance().folders();
        if ((node != filesNode || node != foldersNode)
                && node->type == Node::File) {
            SendEvents::treeViewDoublueClicked(node->toolTip, TreeProxy::instance().rootFromFolder(node)->toolTip);
        }
    }
}

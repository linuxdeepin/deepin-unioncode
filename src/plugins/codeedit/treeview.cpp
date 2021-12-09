#include "treeview.h"
#include "treemodel.h"
#include "treeproxy.h"
#include "treemenu.h"
#include "treeviewdelegate.h"

#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

class TreeViewPrivate
{
    friend class TreeView;
    TreeModel *model;
    TreeViewDelegate *delegate;
    const QString RENAME = "Rename";
    const QString COLSE_ALL = "Colse All";
    const QString COLSE_THE_FILE = "Colse The File";
    const QString COLSE_THE_FOLDER = "Colse The Folder";
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
                     this, &TreeView::selectAppended);

    QObject::connect(&TreeProxy::instance(), &TreeProxy::appendedFolder,
                     this, &TreeView::selectAppended);

    QObject::connect(d->model, &TreeModel::renamedChild,
                     this, &TreeView::selectRenamed, Qt::QueuedConnection);
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
            QAction *colseAllAction = new QAction(d->COLSE_ALL, &menu);
            QAction::connect(colseAllAction, &QAction::triggered, this, [=](){
                this->closeAll(node); // close all node
            });
            menu.addAction(colseAllAction);
        }

        if (node->parent == filesNode) {
            QAction *colseAllAction = new QAction(d->COLSE_THE_FILE, &menu);
            QAction::connect(colseAllAction, &QAction::triggered, colseAllAction, [=](){
                this->closeTheFile(node); // close the file
            });
            menu.addAction(colseAllAction);
        }

        if (node->parent == foldersNode) {
            QAction *colseAllAction = new QAction(d->COLSE_THE_FOLDER, &menu);
            QAction::connect(colseAllAction, &QAction::triggered, colseAllAction, [=](){
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
        }

        if ((node != filesNode && node != foldersNode)) {
            menu.createOptions(node->toolTip); // file options
        }
        menu.exec();
    }
}

void TreeView::selectAppended(Node *node)
{
    if (!node)
        return;

    QModelIndex expandIndex = d->model->index(TreeProxy::instance().root(node));
    this->expand(expandIndex);
    selectionModel()->select(d->model->index(node), QItemSelectionModel::ClearAndSelect);
}

void TreeView::selectRenamed(Node *node)
{
    if (!node)
        return;

    selectionModel()->clear();
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
            emit clickedFile(node->toolTip);
        }
    }
}

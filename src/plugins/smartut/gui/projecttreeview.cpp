// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projecttreeview.h"
#include "common/projectitemmodel.h"
#include "common/projectitemdelegate.h"
#include "utils/utils.h"

#include "common/util/eventdefinitions.h"

#include <DDesktopServices>

#include <QMenu>
#include <QHeaderView>
#include <QContextMenuEvent>

DWIDGET_USE_NAMESPACE

class ProjectTreeViewPrivate : public QObject
{
public:
    explicit ProjectTreeViewPrivate(ProjectTreeView *qq);

    void initUI();
    void initConnecttion();

    void setItemIgnoreState(NodeItem *item, bool ignore);
    void createUTActoins(QMenu *menu, NodeItem *item);

public:
    ProjectTreeView *q;

    ProjectItemModel *model { nullptr };
    ProjectTreeView::ViewType viewType;
};

ProjectTreeViewPrivate::ProjectTreeViewPrivate(ProjectTreeView *qq)
    : q(qq)
{
}

void ProjectTreeViewPrivate::initUI()
{
    model = new ProjectItemModel(q);
    q->setModel(model);
    q->setItemDelegate(new ProjectItemDelegate(q));

    q->setLineWidth(0);
    q->setContentsMargins(0, 0, 0, 0);
    q->setFrameShape(QFrame::NoFrame);
    q->setIconSize(QSize(16, 16));

    q->setRootIsDecorated(false);
    q->setIndentation(0);
    q->setMouseTracking(true);
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    q->setTextElideMode(Qt::ElideNone);
    q->setHeaderHidden(true);
    q->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    q->header()->setStretchLastSection(true);
}

void ProjectTreeViewPrivate::initConnecttion()
{
    if (viewType == ProjectTreeView::UnitTest) {
        connect(q, &ProjectTreeView::doubleClicked, this,
                [this](const QModelIndex &index) {
                    auto node = model->nodeForIndex(index);
                    if (node && node->isFileNodeType() && QFile::exists(node->filePath()))
                        editor.openFile(QString(), node->filePath());
                });
    }
}

void ProjectTreeViewPrivate::setItemIgnoreState(NodeItem *item, bool ignore)
{
    if (item->itemNode->isFileNodeType()) {
        item->state = ignore ? Ignored : None;
        q->updateItem(item);
    } else if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            setItemIgnoreState(dynamic_cast<NodeItem *>(item->child(i)), ignore);
        }
    }
}

void ProjectTreeViewPrivate::createUTActoins(QMenu *menu, NodeItem *item)
{
    if (item->itemNode->isFileNodeType()) {
        auto act = menu->addAction(ProjectTreeView::tr("Open File"), this, [item] {
            editor.openFile(QString(), item->itemNode->filePath());
        });
        act->setEnabled(QFile::exists(item->itemNode->filePath()));
    }

    bool allIgnored = Utils::checkAllState(item, Ignored);
    QAction *act = nullptr;
    if (Utils::checkAllState(item, Completed))
        act = menu->addAction(ProjectTreeView::tr("Regenerate"), this, std::bind(&ProjectTreeView::reqGenerateUTFile, q, item));
    else if (!item->itemNode->isFileNodeType() && Utils::checkAnyState(item, Completed))
        act = menu->addAction(ProjectTreeView::tr("Continue To Generate"), this, std::bind(&ProjectTreeView::reqContinueToGenerate, q, item));
    else
        act = menu->addAction(ProjectTreeView::tr("Generate"), this, std::bind(&ProjectTreeView::reqGenerateUTFile, q, item));
    act->setEnabled(!allIgnored);

    bool started = Utils::checkAnyState(item, Generating);
    if (started)
        menu->addAction(ProjectTreeView::tr("Stop"), this, std::bind(&ProjectTreeView::reqStopGenerate, q, item));

    if (allIgnored)
        menu->addAction(ProjectTreeView::tr("Unignore"), this, std::bind(&ProjectTreeViewPrivate::setItemIgnoreState, this, item, false));
    else
        menu->addAction(ProjectTreeView::tr("Ignore"), this, std::bind(&ProjectTreeViewPrivate::setItemIgnoreState, this, item, true));

    act = menu->addAction(ProjectTreeView::tr("Show Containing Folder"), this, [item] {
        DDesktopServices::showFileItem(item->itemNode->filePath());
    });
    act->setEnabled(QFile::exists(item->itemNode->filePath()));
}

ProjectTreeView::ProjectTreeView(ViewType type, QWidget *parent)
    : DTreeView(parent),
      d(new ProjectTreeViewPrivate(this))
{
    d->viewType = type;
    d->initUI();
    d->initConnecttion();
}

ProjectTreeView::ViewType ProjectTreeView::viewType() const
{
    return d->viewType;
}

void ProjectTreeView::setRootProjectNode(ProjectNode *rootNode)
{
    d->model->setRootProjectNode(rootNode);
}

NodeItem *ProjectTreeView::rootItem() const
{
    return d->model->rootItem();
}

void ProjectTreeView::clear()
{
    d->model->clear();
}

void ProjectTreeView::updateItem(NodeItem *item)
{
    auto index = d->model->indexFromItem(item);
    if (index.isValid())
        update(index);
}

void ProjectTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    auto item = d->model->itemForIndex(index);
    if (!item)
        return;

    QMenu menu;
    if (d->viewType == UnitTest)
        d->createUTActoins(&menu, item);

    if (!item->itemNode->isFileNodeType()) {
        menu.addSeparator();
        if (isExpanded(index))
            menu.addAction(tr("Collapse"), this, std::bind(&ProjectTreeView::collapse, this, index));
        else
            menu.addAction(tr("Expand"), this, std::bind(&ProjectTreeView::expand, this, index));
        menu.addAction(tr("Collapse All"), this, &ProjectTreeView::collapseAll);
        menu.addAction(tr("Expand All"), this, &ProjectTreeView::expandAll);
    }

    if (!menu.actions().isEmpty())
        menu.exec(QCursor::pos());
}

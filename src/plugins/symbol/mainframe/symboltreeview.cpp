// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symboltreeview.h"
#include "symbolmodel.h"
#include "definitions.h"

#include "services/project/projectservice.h"

#include "symboldelegate.h"

#include <QHeaderView>
#include <QDirIterator>
#include <QStandardItemModel>

class SymbolTreeViewPrivate
{
    friend class SymbolTreeView;
    SymbolTreeView *const q;
    QModelIndex selIndex;
    QFileSystemModel *model {nullptr};
    SymbolDelegate *delegate;
    QMenu *getFileLineMenu(const QString &filePath);
    SymbolTreeViewPrivate(SymbolTreeView *qq): q(qq){}
};

SymbolTreeView::SymbolTreeView(QWidget *parent)
    : QTreeView(parent)
    , d (new SymbolTreeViewPrivate(this))
{
    d->model = new SymbolModel();
    d->delegate = new SymbolDelegate;
    QTreeView::setModel(d->model);
    QTreeView::setItemDelegate(d->delegate);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QTreeView::NoEditTriggers);	          //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		  //一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框
    header()->setVisible(false);
    QObject::connect(this, &QTreeView::doubleClicked,
                     this, &SymbolTreeView::doDoubleClieked,
                     Qt::UniqueConnection);
    QObject::connect(this, &QTreeView::customContextMenuRequested,
                     this, &SymbolTreeView::doContextMenu,
                     Qt::UniqueConnection);
}

SymbolTreeView::~SymbolTreeView()
{
    if (d)
        delete d;
}

void SymbolTreeView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
    setRootIndex(d->model->index(filePath));
}

void SymbolTreeView::doDoubleClieked(const QModelIndex &index)
{
    Q_UNUSED(index)
    // nothing to do
}

void SymbolTreeView::doContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (!index.isValid())
        return;

    setCurrentIndex(index);

    QMenu *contextMenu{nullptr};
    if (d->model->isDir(index)) {
        QString filePath = d->model->filePath(index);
        QDir currDir(filePath);
        currDir.setFilter(QDir::Files|QDir::Hidden);
        currDir.setSorting(QDir::Name);
        QStringList files = currDir.entryList();
        qInfo() << files;

        if (files.contains(SymbolPri::definitionsFileName)
                || files.contains(SymbolPri::declaredFileName)) {
            contextMenu = new QMenu(this);
        }

        if (files.contains(SymbolPri::definitionsFileName)) {
            QAction *newAction = new QAction(SymbolPri::definitionsAcStr, contextMenu);
            contextMenu->addAction(newAction);
            auto defJumpMenu = d->getFileLineMenu(filePath + QDir::separator() + SymbolPri::definitionsFileName);
            if (defJumpMenu) {
                newAction->setMenu(defJumpMenu);
            }
        }

        if (files.contains(SymbolPri::declaredFileName)) {
            QAction *newAction = new QAction(SymbolPri::declaredAcStr, contextMenu);
            contextMenu->addAction(newAction);
            auto declJumpMenu = d->getFileLineMenu(filePath + QDir::separator() + SymbolPri::declaredFileName);
            if (declJumpMenu) {
                newAction->setMenu(declJumpMenu);
            }
        }
    }

    if (contextMenu) {
        contextMenu->exec(QWidget::mapToGlobal(point));
        delete contextMenu;
    }
}

QMenu *SymbolTreeViewPrivate::getFileLineMenu(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << file.errorString();
    }
    QMenu *defMenu = nullptr;
    QStringList lines = QString(file.readAll()).split('\n');
    for (auto line : lines) {
        if (!line.isEmpty()) {
            if (!defMenu) {
                defMenu = new QMenu();
            }
            QAction *action = new QAction(defMenu);
            QObject::connect(action, &QAction::triggered, [=](){
                QStringList jumpLists = action->text().split(":");
                if (jumpLists.size() >= 2) {
                    q->jumpToLine(jumpLists[0], jumpLists[1]);
                }
            });
            action->setText(line);
            defMenu->addAction(action);
        }
    }
    file.close();
    return defMenu;
}

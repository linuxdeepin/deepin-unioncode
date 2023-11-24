// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symboltreeview.h"
#include "symbolmodel.h"
#include "definitions.h"

#include "services/project/projectservice.h"

#include <DMenu>
#include <DTreeView>
#include <DWidget>

#include <QHeaderView>
#include <QDirIterator>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE
class SymbolTreeViewPrivate
{
    friend class SymbolTreeView;
    SymbolTreeView *const q;
    QModelIndex selIndex;
    QFileSystemModel *model {nullptr};
    DMenu *getFileLineMenu(const QString &filePath);
    SymbolTreeViewPrivate(SymbolTreeView *qq): q(qq){}
};

SymbolTreeView::SymbolTreeView(DWidget *parent)
    : DTreeView(parent)
    , d (new SymbolTreeViewPrivate(this))
{
    d->model = new SymbolModel();
    DTreeView::setModel(d->model);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(DTreeView::NoEditTriggers);	          //节点不能编辑
    setSelectionBehavior(DTreeView::SelectRows);		  //一次选中整行
    setSelectionMode(DTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    header()->setVisible(false);

    QObject::connect(this, &DTreeView::doubleClicked,
                     this, &SymbolTreeView::doDoubleClieked,
                     Qt::UniqueConnection);
    QObject::connect(this, &DTreeView::customContextMenuRequested,
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

    DMenu *contextMenu{nullptr};
    if (d->model->isDir(index)) {
        QString filePath = d->model->filePath(index);
        QDir currDir(filePath);
        currDir.setFilter(QDir::Files|QDir::Hidden);
        currDir.setSorting(QDir::Name);
        QStringList files = currDir.entryList();
        qInfo() << files;

        if (files.contains(SymbolPri::definitionsFileName)
                || files.contains(SymbolPri::declaredFileName)) {
            contextMenu = new DMenu(this);
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
        contextMenu->exec(DWidget::mapToGlobal(point));
        delete contextMenu;
    }
}

DMenu *SymbolTreeViewPrivate::getFileLineMenu(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << file.errorString();
    }
    DMenu *defMenu = nullptr;
    QStringList lines = QString(file.readAll()).split('\n');
    for (auto line : lines) {
        if (!line.isEmpty()) {
            if (!defMenu) {
                defMenu = new DMenu();
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

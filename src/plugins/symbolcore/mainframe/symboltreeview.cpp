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
#include "symboltreeview.h"

#include "transceiver/sendevents.h"

#include "services/project/projectservice.h"

#include "symboldelegate.h"

#include <QHeaderView>
#include <QStandardItemModel>

class SymbolTreeViewPrivate
{
    friend class SymbolTreeView;
    libClang::Location selLocation;
    QModelIndex selIndex;
    QStandardItemModel *model {nullptr};
    SymbolDelegate *delegate;
};

SymbolTreeView::SymbolTreeView(QWidget *parent)
    : QTreeView(parent)
    , d (new SymbolTreeViewPrivate)
{
    d->model = new QStandardItemModel;
    d->delegate = new SymbolDelegate;
    QTreeView::setModel(d->model);
    QTreeView::setItemDelegate(d->delegate);

    setEditTriggers(QTreeView::NoEditTriggers);	          //节点不能编辑
    setSelectionBehavior(QTreeView::SelectRows);		  //一次选中整行
    setSelectionMode(QTreeView::SingleSelection);         //单选，配合上面的整行就是一次选单行
    setFocusPolicy(Qt::NoFocus);                          //去掉鼠标移到节点上时的虚线框
    this->header()->hide();
    QObject::connect(this , &QTreeView::doubleClicked,
                     this, &SymbolTreeView::doDoubleClieked,
                     Qt::UniqueConnection);
}

SymbolTreeView::~SymbolTreeView()
{
    if (d)
        delete d;
}

void SymbolTreeView::appendRoot(QStandardItem *root)
{
    d->model->appendRow(root);
}

void SymbolTreeView::activeRoot(QStandardItem *root)
{
    if (!root && root != SymbolTreeView::root(root))
        return;
    d->delegate->setActiveProject(d->model->indexFromItem(root));
}

void SymbolTreeView::deleteRoot(QStandardItem *root)
{
    if (!root && root != SymbolTreeView::root(root))
        return;

    this->removeRootItem(root);
}

QStandardItem *SymbolTreeView::itemFromProjectInfo(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    for(int row = 0; row < d->model->rowCount(); row++) {
        auto item = d->model->item(row);
        if (info == ProjectInfo::get(item)) {
            return item;
        }
    }
    return nullptr;
}

void SymbolTreeView::removeRootItem(QStandardItem *root)
{
    using namespace dpfservice;
    auto info = ProjectInfo::get(ProjectGenerator::root(root));

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

    QString workspaceFolder = info.workspaceFolder();
    QString language = info.language();
    if (!workspaceFolder.isEmpty() && !language.isEmpty() && isDeleted) {
        // 停止lspClient
        lsp::ClientManager::instance()->shutdownClient({workspaceFolder, language});
    }

    int rowCount = d->model->rowCount();
    if ( 0 < rowCount) { // 存在其他工程时
        // 始终保持首选项
        auto index = d->model->index(0, 0);
        activeRoot(d->model->itemFromIndex(index));
    }
}

QStandardItem *SymbolTreeView::itemFromIndex(const QModelIndex &index)
{
    return d->model->itemFromIndex(index);
}

void SymbolTreeView::doDoubleClieked(const QModelIndex &index)
{
    using namespace dpfservice;
    if (!index.isValid())
        return;

    auto proInfo = ProjectInfo::get(root(index));
    QString language = proInfo.language();
    QString workspace = proInfo.workspaceFolder();

    if (d->selIndex == index) {
        auto locations = SymbolInfo::get(index).getLocations();
        auto itera = locations.begin();
        while (itera != locations.end()) {
            if (* itera == d->selLocation) {

                if (++ itera == locations.end()) {
                    itera = locations.begin();
                }
                QString filePath = itera->getFilePath();
                int line = itera->getLine();
                // send jump to line
                SendEvents::doubleCliekedJumpToLine(language, workspace, filePath, line);

                break;
            }
            ++ itera;
        }
    } else {
        auto locations = SymbolInfo::get(index).getLocations();
        if (locations.size() > 0) {
            auto location = locations.toList()[0];
            // send jump to line
            SendEvents::doubleCliekedJumpToLine(language, workspace,
                                                location.getFilePath(),
                                                location.getLine());
        }
    }
}

QModelIndex SymbolTreeView::root(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto curr = index;
    while (curr.parent().isValid()) {
        curr = curr.parent();
    }
    return curr;
}

QStandardItem *SymbolTreeView::root(const QStandardItem *item) const
{
    if (!item)
        return nullptr;
    auto curr = item;
    while (curr->parent()) {
        curr = curr->parent();
    }
    return const_cast<QStandardItem*>(curr) ;
}

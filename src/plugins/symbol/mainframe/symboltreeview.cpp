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
#include "symbolmodel.h"

#include "services/project/projectservice.h"

#include "symboldelegate.h"

#include <QHeaderView>
#include <QStandardItemModel>

class SymbolTreeViewPrivate
{
    friend class SymbolTreeView;
    QModelIndex selIndex;
    QFileSystemModel *model {nullptr};
    SymbolDelegate *delegate;
};

SymbolTreeView::SymbolTreeView(QWidget *parent)
    : QTreeView(parent)
    , d (new SymbolTreeViewPrivate)
{
    d->model = new SymbolModel();
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

void SymbolTreeView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
}

void SymbolTreeView::doDoubleClieked(const QModelIndex &index)
{
//    using namespace dpfservice;
//    if (!index.isValid())
//        return;

//    auto proInfo = ProjectInfo::get(root(index));
//    QString language = proInfo.language();
//    QString workspace = proInfo.workspaceFolder();

//    if (d->selIndex == index) {
//        auto locations = SymbolInfo::get(index).getLocations();
//        auto itera = locations.begin();
//        while (itera != locations.end()) {
//            if (* itera == d->selLocation) {

//                if (++ itera == locations.end()) {
//                    itera = locations.begin();
//                }
//                QString filePath = itera->getFilePath();
//                int line = itera->getLine();
//                // send jump to line
//                SendEvents::doubleCliekedJumpToLine(language, workspace, filePath, line);

//                break;
//            }
//            ++ itera;
//        }
//    } else {
//        auto locations = SymbolInfo::get(index).getLocations();
//        if (locations.size() > 0) {
//            auto location = locations.toList()[0];
//            // send jump to line
//            SendEvents::doubleCliekedJumpToLine(language, workspace,
//                                                location.getFilePath(),
//                                                location.getLine());
//        }
//    }
}

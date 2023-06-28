// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeviewkeeper.h"
#include "treeview.h"

TreeViewKeeper::TreeViewKeeper()
{

}

TreeViewKeeper *TreeViewKeeper::instance()
{
    static TreeViewKeeper ins;
    return &ins;
}

TreeView *TreeViewKeeper::treeView()
{
    if (!iTreeView) {
        iTreeView  = new TreeView;
    }
    return iTreeView;
}

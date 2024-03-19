// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREEVIEWKEEPER_H
#define TREEVIEWKEEPER_H

#include "filetreeview.h"

class TreeViewKeeper final
{
    TreeViewKeeper();
    FileTreeView *iTreeView {nullptr};
public:
    static TreeViewKeeper *instance();
    FileTreeView *treeView();
};

#endif // TREEVIEWKEEPER_H

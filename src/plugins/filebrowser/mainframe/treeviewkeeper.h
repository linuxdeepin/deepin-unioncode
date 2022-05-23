#ifndef TREEVIEWKEEPER_H
#define TREEVIEWKEEPER_H

#include "treeview.h"

class TreeViewKeeper final
{
    TreeViewKeeper();
    TreeView *iTreeView {nullptr};
public:
    static TreeViewKeeper *instance();
    TreeView *treeView();
};

#endif // TREEVIEWKEEPER_H

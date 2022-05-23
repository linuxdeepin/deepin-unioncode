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

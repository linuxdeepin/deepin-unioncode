#include "projectkeeper.h"
#include "projecttreeview.h"
#include "common/common.h"

#include <QStandardItem>

namespace {
ProjectTreeView *tree{nullptr};
}

ProjectKeeper::ProjectKeeper()
{

}

ProjectKeeper *ProjectKeeper::instance()
{
    static ProjectKeeper ins;
    return &ins;
}

ProjectTreeView *ProjectKeeper::treeView()
{
    if (!tree)
        tree = new ProjectTreeView();
    return tree;
}


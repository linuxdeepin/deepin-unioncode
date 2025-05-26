// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectkeeper.h"
#include "projecttree.h"
#include "common/common.h"

#include <QStandardItem>

namespace {
ProjectTree *tree{nullptr};
}

ProjectKeeper::ProjectKeeper()
{

}

ProjectKeeper *ProjectKeeper::instance()
{
    static ProjectKeeper ins;
    return &ins;
}

ProjectTree *ProjectKeeper::treeView()
{
    if (!tree)
        tree = new ProjectTree();
    return tree;
}




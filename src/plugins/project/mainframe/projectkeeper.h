// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTKEEPER_H
#define PROJECTKEEPER_H

#include "services/project/projectservice.h"

#include <QObject>

class QAction;
class ProjectTree;
class QStandardItem;
class ProjectKeeper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectKeeper)
public:
    ProjectKeeper();
    static ProjectKeeper *instance();
    ProjectTree *treeView();
};

#endif // PROJECTKEEPER_H

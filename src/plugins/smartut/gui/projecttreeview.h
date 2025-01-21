// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include "common/itemnode.h"

#include <DTreeView>

class ProjectTreeViewPrivate;
class ProjectTreeView : public DTK_WIDGET_NAMESPACE::DTreeView
{
    Q_OBJECT
public:
    enum ViewType {
        Project,
        UnitTest
    };

    explicit ProjectTreeView(ViewType type, QWidget *parent = nullptr);

    ViewType viewType() const;
    void setRootProjectNode(ProjectNode *rootNode);
    void updateProjectNode(ProjectNode *prjNode);
    NodeItem *rootItem() const;

    void clear();

public Q_SLOTS:
    void updateItem(NodeItem *item);

Q_SIGNALS:
    void reqGenerateUTFile(NodeItem *item);
    void reqContinueToGenerate(NodeItem *item);
    void reqStopGenerate(NodeItem *item);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    ProjectTreeViewPrivate *const d;
};

#endif   // PROJECTTREEVIEW_H

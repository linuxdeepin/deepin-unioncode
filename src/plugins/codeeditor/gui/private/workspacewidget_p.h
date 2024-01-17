// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_P_H
#define WORKSPACEWIDGET_P_H

#include "gui/workspacewidget.h"
#include "gui/tabwidget.h"

class WorkspaceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WorkspaceWidgetPrivate(WorkspaceWidget *qq);

    void initUI();
    void initConnection();
    void connectTabWidgetSignals(TabWidget *tabWidget);

    TabWidget *currentTabWidget() const;
    void doSplit(QSplitter *spliter, int index, const QString &fileName, int pos, int scroll);

public:
    void onSplitRequested(Qt::Orientation ori, const QString &fileName);
    void onCloseRequested();
    void onOpenFileRequested(const QString &fileName);
    void onFocusChanged(QWidget *old, QWidget *now);

public:
    WorkspaceWidget *q;

    TabWidget *focusTabWidget { nullptr };
    QList<TabWidget *> tabWidgetList;
};

#endif   // WORKSPACEWIDGET_P_H

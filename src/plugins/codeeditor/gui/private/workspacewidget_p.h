// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_P_H
#define WORKSPACEWIDGET_P_H

#include "gui/workspacewidget.h"
#include "gui/tabwidget.h"

#include "common/util/eventdefinitions.h"

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
    void onFocusChanged(QWidget *old, QWidget *now);
    void onZoomValueChanged();

    void handleOpenFile(const QString &workspace, const QString &fileName);
    void handleAddBreakpoint(const QString &fileName, int line);
    void handleRemoveBreakpoint(const QString &fileName, int line);
    void handleBack();
    void handleForward();
    void handleSetDebugLine(const QString &fileName, int line);
    void handleRemoveDebugLine();
    void handleGotoLine(const QString &fileName, int line);
    void handleGotoPosition(const QString &fileName, int line, int column);

public:
    WorkspaceWidget *q;

    TabWidget *focusTabWidget { nullptr };
    QList<TabWidget *> tabWidgetList;

    int zoomValue { 0 };
};

#endif   // WORKSPACEWIDGET_P_H

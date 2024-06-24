// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_P_H
#define TABBAR_P_H

#include "gui/tabbar.h"

#include <DTabBar>
#include <DToolButton>

DWIDGET_USE_NAMESPACE

class TabBarPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TabBarPrivate(TabBar *qq);

    void initUI();
    void initConnection();

    void updateBackgroundColor();
    bool isModified(int index) const;
    int showConfirmDialog(const QString &filePath);
    void closeAllTab(const QStringList &exceptList);

public slots:
    void onCurrentTabChanged(int index);
    void onTabColseRequested(int index);
    void showMenu(QPoint pos);

public:
    TabBar *q;

    DTabBar *tabBar { nullptr };
    DToolButton *hSplitBtn { nullptr };
    DToolButton *vSplitBtn { nullptr };
    DToolButton *closeBtn { nullptr };
};

#endif   // TABBAR_P_H

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABWIDGET_P_H
#define TABWIDGET_P_H

#include "gui/tabwidget.h"
#include "gui/tabbar.h"
#include "manager/texteditormanager.h"

#include <QStackedLayout>

class TabWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TabWidgetPrivate(TabWidget *qq);

    void initUI();
    void initConnection();

    TextEditor *currentTextEditor() const;
    void changeFocusProxy();

public:
    void onTabSwitched(const QString &fileName);
    void onTabClosed(const QString &fileName);
    void onSpliterClicked(Qt::Orientation ori);

public:
    TabWidget *q;

    QStackedLayout *editorLayout { nullptr };
    TabBar *tabBar { nullptr };
    TextEditorManager *editorMng { nullptr };

    QHash<QString, int> editorIndexHash;
};

#endif   // TABWIDGET_P_H

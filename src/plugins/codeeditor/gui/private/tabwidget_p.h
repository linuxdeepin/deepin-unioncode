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
    struct PosRecord
    {
        int pos = 0;
        QString fileName;
    };

    explicit TabWidgetPrivate(TabWidget *qq);

    void initUI();
    void initConnection();

    TextEditor *currentTextEditor() const;
    void changeFocusProxy();
    bool processKeyPressEvent(QKeyEvent *event);

    void doSave();
    void removePositionRecord(const QString &fileName);

public:
    void onTabSwitched(const QString &fileName);
    void onTabClosed(const QString &fileName);
    void onSpliterClicked(Qt::Orientation ori);
    void onLinePositionChanged(int line, int index);

public:
    TabWidget *q;

    QStackedLayout *editorLayout { nullptr };
    TabBar *tabBar { nullptr };
    TextEditorManager *editorMng { nullptr };

    QHash<QString, int> editorIndexHash;

    PosRecord curPosRecord;
    QList<PosRecord> prePosRecord;
    QList<PosRecord> nextPosRecord;
};

#endif   // TABWIDGET_P_H

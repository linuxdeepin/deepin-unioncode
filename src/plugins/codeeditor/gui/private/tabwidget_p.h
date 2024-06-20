// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABWIDGET_P_H
#define TABWIDGET_P_H

#include "gui/tabwidget.h"
#include "gui/tabbar.h"
#include "gui/texteditor.h"

#include "common/util/eventdefinitions.h"

#include <QStackedLayout>

class TabWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    struct PosRecord
    {
        int pos = 0;
        QString fileName;

        bool operator==(const PosRecord &other)
        {
            return pos == other.pos && fileName == other.fileName;
        }
    };

    explicit TabWidgetPrivate(TabWidget *qq);

    void initUI();
    void initConnection();
    QWidget *createSpaceWidget();
    QWidget *createFindPlaceHolder();

    TextEditor *createEditor(const QString &fileName = "");
    TextEditor *findEditor(const QString &fileName);
    TextEditor *currentTextEditor() const;
    void changeFocusProxy();
    bool processKeyPressEvent(QKeyEvent *event);
    void replaceRange(const QString &fileName, const newlsp::Range &range, const QString &text);

    void doSave();
    void removePositionRecord(const QString &fileName);

public slots:
    void onTabSwitched(const QString &fileName);
    void onTabClosed(const QString &fileName);
    void onSpliterClicked(Qt::Orientation ori);
    void onCursorRecordChanged(int pos);
    void onFileChanged(const QString &fileName);
    void handleAddAnnotation(const QString &fileName, const QString &title, const QString &content, int line, AnnotationType type);
    void handleRemoveAnnotation(const QString &fileName, const QString &title);
    void handleClearAllAnnotation(const QString &title);
    void handleSetLineBackgroundColor(const QString &fileName, int line, const QColor &color);
    void handleResetLineBackground(const QString &fileName, int line);
    void handleClearLineBackground(const QString &fileName);
    void handleDoRename(const newlsp::WorkspaceEdit &info);

public:
    TabWidget *q;

    QStackedLayout *editorLayout { nullptr };
    TabBar *tabBar { nullptr };
    QHash<QString, TextEditor *> editorMng;

    PosRecord curPosRecord;
    QList<PosRecord> prePosRecord;
    QList<PosRecord> nextPosRecord;
};

#endif   // TABWIDGET_P_H

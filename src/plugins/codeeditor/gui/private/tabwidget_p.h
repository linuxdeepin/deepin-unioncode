// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABWIDGET_P_H
#define TABWIDGET_P_H

#include "gui/tabwidget.h"
#include "gui/tabbar.h"
#include "gui/texteditor.h"
#include "gui/recent/recentopenwidget.h"
#include "find/editordocumentfind.h"
#include "symbol/symbolbar.h"

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

    TextEditor *createEditor(const QString &fileName = "", QsciDocument *doc = nullptr);
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
    void handleDoRename(const newlsp::WorkspaceEdit &info);
    void handleOpenFiles(const QList<QUrl> &fileList);

public:
    TabWidget *q;

    QStackedLayout *editorLayout { nullptr };
    TabBar *tabBar { nullptr };
    SymbolBar *symbolBar { nullptr };
    QHash<QString, TextEditor *> editorMng;

    PosRecord curPosRecord;
    QList<PosRecord> prePosRecord;
    QList<PosRecord> nextPosRecord;

    QVector<QString> recentOpenedFiles;
    RecentOpenWidget *openedWidget { nullptr };
    EditorDocumentFind *docFind { nullptr };
};

#endif   // TABWIDGET_P_H

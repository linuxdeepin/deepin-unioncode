// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include "gui/texteditor.h"
#include "common/util/eventdefinitions.h"
#include "lsp/lspstyle.h"
#include "gui/completion/codecompletionwidget.h"

#include <Qsci/qscistyle.h>

class TextEditorPrivate : public QObject
{
    Q_OBJECT
public:
    enum MarginType {
        LineNumberMargin = 0,
        SymbolMargin,
        ChangeBarMargin,
        FoldingMargin
    };

    enum MarkerNumber {
        Breakpoint = 0,
        BreakpointDisabled,
        Bookmark,
        Runtime,
        RuntimeLineBackground,
        CustomLineBackground
    };

    explicit TextEditorPrivate(TextEditor *qq);

    void init();
    void initConnection();
    void initMargins();
    void updateColorTheme();
    void loadLexer();
    void loadLSPStyle();

    int cursorPosition() const;
    int marginsWidth();
    void setMarginVisible(MarginType type, bool visible);
    void updateLineNumberMargin(bool visible);

    void showContextMenu();
    void showMarginMenu();

    void gotoNextMark(uint mask);
    void gotoPreviousMark(uint mask);
    QsciStyle createAnnotationStyle(int type);
    void adjustScrollBar();

public slots:
    void resetThemeColor();
    void onDwellStart(int position, int x, int y);
    void onDwellEnd(int position, int x, int y);
    void onModified(int pos, int mtype, const QString &text, int len, int added,
                    int line, int foldNow, int foldPrev, int token, int annotationLinesAdded);
    void updateSettings();

public:
    TextEditor *q { nullptr };

    QString fileName;
    int preFirstLineNum { 0 };
    int lastCursorPos { 0 };
    QMultiHash<QString, int> annotationRecords;

    LSPStyle *lspStyle { nullptr };
    bool isSaved { false };
    bool isAutoCompletionEnabled { false };
    bool isCtrlPressed { false };

    bool contentsChanged { false };
    bool lastCursorNeedRecord { true };
    bool postionChangedByGoto { false };
    QString fontName;
    int fontSize { 10 };

    CodeCompletionWidget *completionWidget { nullptr };
    
    QMap<QString, QVariant> commentSettings;
};

#endif   // TEXTEDITOR_P_H

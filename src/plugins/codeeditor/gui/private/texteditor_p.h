// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include "gui/texteditor.h"

class TextEditorPrivate : public QObject
{
    Q_OBJECT
public:
    enum MarkSymbol {
        BreakpointSymbol = 0,
        RuntimeSymbol,
        BookmarkSymbol
    };

    enum MarginType {
        LineNumberMargin = 0,
        SymbolMargin,
        ChangeBarMargin,
        FoldingMargin
    };

    enum MarginMask {
        BreakpointMask = 1,
        BreakpointDisabledMask = 1 << 1,
        BookmarkMask = 1 << 2,
        RuntimeMask = 1 << 3,
        WarningMask = 1 << 4,
        ErrorMask = 1 << 5,

        ChangeUnsavedMask = 1 << 6,
        ChangeSavedMask = 1 << 7
    };

    explicit TextEditorPrivate(TextEditor *qq);

    void init();
    void initConnection();
    void initMargins();
    void updateColorTheme();
    void updateSettings();

    void loadDefaultLexer();

    int cursorPosition() const;
    int marginsWidth();
    void setMarginVisible(MarginType type, bool visible);
    void updateLineNumberMargin(bool visible);

    QVariantHash getMenuParams(QContextMenuEvent *event);

    void gotoNextMark(MarginMask mask);
    void gotoPreviousMark(MarginMask mask);

public:
    TextEditor *q { nullptr };

    QString fileName;
    int preFirstLineNum { 0 };
    int lastCursorPos { 0 };
};

#endif   // TEXTEDITOR_P_H

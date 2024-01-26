// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include "gui/texteditor.h"
#include "common/util/eventdefinitions.h"

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
    void updateSettings();
    void loadLexer();

    int cursorPosition() const;
    int marginsWidth();
    void setMarginVisible(MarginType type, bool visible);
    void updateLineNumberMargin(bool visible);

    void showContextMenu();
    void showMarginMenu();

    void gotoNextMark(uint mask);
    void gotoPreviousMark(uint mask);
    bool doFind(const QString &keyword, bool isForward);
    QsciStyle createAnnotationStyle(int type);

public slots:
    void onThemeTypeChanged();
    void handleSearch(const QString &keyword, int operateType);
    void handleReplace(const QString &srcText, const QString &destText, int operateType);

public:
    TextEditor *q { nullptr };

    QString fileName;
    int preFirstLineNum { 0 };
    int lastCursorPos { 0 };
    QMultiHash<QString, int> annotationRecords;
};

#endif   // TEXTEDITOR_P_H

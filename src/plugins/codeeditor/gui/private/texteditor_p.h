// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include "gui/texteditor.h"
#include "common/util/eventdefinitions.h"
#include "lsp/languageclienthandler.h"
#include "gui/completion/codecompletionwidget.h"

#include <Qsci/qscistyle.h>

#include <DFloatingWidget>

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
        RuntimeLineBackground
    };

    explicit TextEditorPrivate(TextEditor *qq);

    void init();
    void initConnection();
    void initMargins();
    void initWidgetContainer();
    void updateColorTheme();
    void loadLexer();
    void initLanguageClient();

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
    QMap<int, int> allMarkers();
    void setMarkers(const QMap<int, int> &maskMap);

    QWidget *mainWindow();
    void setContainerWidget(QWidget *widget);
    void updateLineWidgetPosition();

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
    QMultiHash<QString, int> eOLAnnotationRecords;

    LanguageClientHandler *languageClient { nullptr };
    bool isAutoCompletionEnabled { false };

    bool tipsDisplayable { true };
    bool contentsChanged { false };
    bool lastCursorNeedRecord { true };
    bool postionChangedByGoto { false };
    QString fontName;
    int fontSize { 10 };

    using CompletionCache = QPair<int, QString>;
    CompletionCache cpCache { -1, "" };
    CodeCompletionWidget *completionWidget { nullptr };
    QMap<QString, QVariant> commentSettings;

    std::tuple<int, int, int, int> selectionCache { -1, -1, -1, -1 };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *lineWidgetContainer { nullptr };
    int showAtLine { 0 };
};

#endif   // TEXTEDITOR_P_H

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor_p.h"
#include "utils/editorutils.h"

#include <Qsci/qsciapis.h>

#include <DGuiApplicationHelper>

#include <QDebug>

static constexpr int MARGIN_SYMBOLE_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_FOLDER_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_CHANGEBAR_DEFAULT_WIDTH = 3;

static constexpr int INDENTATION_DEFAULT_WIDTH = 4;

DGUI_USE_NAMESPACE

TextEditorPrivate::TextEditorPrivate(TextEditor *qq)
    : q(qq)
{
    init();
}

void TextEditorPrivate::init()
{
    initMargins();
    updateColorTheme();
    updateSettings();
}

void TextEditorPrivate::initMargins()
{
    // Display line number
    q->setMarginLineNumbers(LineNumberMargin, true);
    setMarginVisible(LineNumberMargin, true);

    // folding
    q->setFolding(TextEditor::BoxedTreeFoldStyle, FoldingMargin);

    // Set the margin symbol
    q->setMarginType(SymbolMargin, TextEditor::SymbolMargin);
    setMarginVisible(SymbolMargin, true);
    q->setMarginSensitivity(SymbolMargin, true);
    q->setMarginMarkerMask(SymbolMargin, BreakpointMask | BreakpointDisabledMask | BookmarkMask | RuntimeMask | WarningMask | ErrorMask);

    // TODO: 使用图片资源代替？
    q->markerDefine(TextEditor::Circle, BreakpointSymbol);
    q->setMarkerBackgroundColor(QColor(Qt::red), BreakpointSymbol);

    q->markerDefine(TextEditor::RightTriangle, BookmarkSymbol);
    q->setMarkerBackgroundColor(QColor(Qt::red), BookmarkSymbol);
}

void TextEditorPrivate::updateColorTheme()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        // editor
        q->setPaper(QColor("#2b2b2b"));
        q->setCaretForegroundColor(QColor("#FFFFFF"));
        q->setCaretLineBackgroundColor(QColor("#444444"));

        // margins
        q->setMarginsBackgroundColor(QColor("#000000"));
        q->setMarginsForegroundColor(QColor("#F8F8F8"));
    } else {
        // editor
        q->setPaper(QColor("#F8F8F8"));
        q->setCaretForegroundColor(QColor("#000000"));
        q->setCaretLineBackgroundColor(QColor("#E8E8FF"));

        // margins
        q->setMarginsBackgroundColor(QColor("#FFFFFF"));
        q->setMarginsForegroundColor(QColor("#b2b2b2"));
    }
}

void TextEditorPrivate::updateSettings()
{
    // Indentation
    q->setIndentationWidth(INDENTATION_DEFAULT_WIDTH);
    q->setAutoIndent(true);
    q->setIndentationsUseTabs(false);

    // Highlight the current line
    q->setCaretLineVisible(true);

    // Opening brace matching
    q->setBraceMatching(TextEditor::SloppyBraceMatch);

    q->setEolMode(TextEditor::EolUnix);
    q->setScrollWidth(1);
    q->setScrollWidthTracking(true);
}

void TextEditorPrivate::loadDefaultLexer()
{
    if (fileName.isEmpty())
        return;

    if (auto lexer = EditorUtils::defaultLexer(fileName)) {
        lexer->setParent(q);
        q->setLexer(lexer);
        setMarginVisible(FoldingMargin, true);
    } else {
        setMarginVisible(FoldingMargin, false);
    }
}

int TextEditorPrivate::cursorPosition() const
{
    return static_cast<int>(q->SendScintilla(TextEditor::SCI_GETCURRENTPOS));
}

int TextEditorPrivate::marginsWidth()
{
    int width = 0;
    auto count = q->margins();
    for (int i = 0; i < count; ++i) {
        width += q->marginWidth(i);
    }

    return width;
}

void TextEditorPrivate::setMarginVisible(TextEditorPrivate::MarginType type, bool visible)
{
    switch (type) {
    case LineNumberMargin:
        updateLineNumberMargin(visible);
        break;
    case SymbolMargin:
        q->setMarginWidth(type, visible ? MARGIN_SYMBOLE_DEFAULT_WIDTH : 0);
        break;
    case FoldingMargin:
        q->setMarginWidth(type, visible ? MARGIN_FOLDER_DEFAULT_WIDTH : 0);
        break;
    case ChangeBarMargin:
        q->setMarginWidth(type, visible ? MARGIN_CHANGEBAR_DEFAULT_WIDTH : 0);
        break;
    }
}

void TextEditorPrivate::updateLineNumberMargin(bool visible)
{
    if (!visible) {
        q->setMarginWidth(LineNumberMargin, 0);
        return;
    }

    q->updateLineNumberWidth(false);
}

QVariantHash TextEditorPrivate::getMenuParams(QContextMenuEvent *event)
{
    QVariantHash params;
    params.insert("id", q->id());
    params.insert("fileName", fileName);
    params.insert("selectedText", q->selectedText());

    int line = 0, index = 0;
    q->getCursorPosition(&line, &index);
    params.insert("cursorLine", line);
    params.insert("cursorIndex", index);

    return params;
}

void TextEditorPrivate::gotoNextMark(MarginMask mask)
{
    int line = q->currentLineNumber() + 1;
    int newLine = q->markerFindNext(line, mask);

    if (-1 == newLine) {
        line = 0;
        newLine = q->markerFindNext(line, mask);
    }

    if (-1 != newLine)
        q->gotoLine(newLine);
}

void TextEditorPrivate::gotoPreviousMark(MarginMask mask)
{
    int line = q->currentLineNumber() - 1;
    int newLine = q->markerFindPrevious(line, mask);

    if (-1 == newLine) {
        line = q->currentLineNumber();
        newLine = q->markerFindPrevious(line, mask);
    }

    if (-1 != newLine)
        q->gotoLine(newLine);
}

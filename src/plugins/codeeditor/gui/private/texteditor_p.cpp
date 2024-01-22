// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor_p.h"
#include "utils/editorutils.h"
#include "utils/colordefine.h"

#include <Qsci/qsciapis.h>

#include <DGuiApplicationHelper>

#include <QMenu>
#include <QDebug>

static constexpr char DEFAULT_FONT_NAME[] { "Courier New" };

static constexpr int MARGIN_SYMBOLE_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_FOLDER_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_CHANGEBAR_DEFAULT_WIDTH = 3;

static constexpr int TAB_DEFAULT_WIDTH = 4;

DGUI_USE_NAMESPACE

TextEditorPrivate::TextEditorPrivate(TextEditor *qq)
    : QObject(qq),
      q(qq)
{
    init();
    initConnection();
}

void TextEditorPrivate::init()
{
    initMargins();
    updateColorTheme();
    updateSettings();
}

void TextEditorPrivate::initConnection()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TextEditorPrivate::updateColorTheme);

    connect(q, SIGNAL(SCN_ZOOM()), q, SIGNAL(zoomValueChanged()));
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
    q->setMarginMarkerMask(SymbolMargin,
                           1 << Breakpoint
                                   | 1 << BreakpointDisabled
                                   | 1 << Bookmark
                                   | 1 << Runtime
                                   | 1 << RuntimeLine
                                   | 1 << Warning
                                   | 1 << Error);

    // TODO: using picture to replace?
    q->markerDefine(TextEditor::Circle, Breakpoint);
    q->setMarkerForegroundColor(EditorColor::Table::get()->FireBrick, Breakpoint);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->FireBrick, Breakpoint);

    q->markerDefine(TextEditor::RightTriangle, Bookmark);
    q->setMarkerBackgroundColor(QColor(Qt::red), Bookmark);

    q->markerDefine(TextEditor::RightArrow, Runtime);
    q->setMarkerForegroundColor(EditorColor::Table::get()->YellowGreen, Runtime);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->YellowGreen, Runtime);

    q->markerDefine(TextEditor::Background, RuntimeLine);
    q->setMarkerForegroundColor(EditorColor::Table::get()->YellowGreen, RuntimeLine);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->YellowGreen, RuntimeLine);
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
    QFont font(DEFAULT_FONT_NAME, 11, QFont::Normal);
    q->setFont(font);
    q->setMarginsFont(font);

    // Indentation
    q->SendScintilla(TextEditor::SCI_SETTABWIDTH, TAB_DEFAULT_WIDTH);
    q->setWhitespaceSize(3);
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
        QFont font(DEFAULT_FONT_NAME, 11, QFont::Normal);
        lexer->setDefaultFont(font);
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

void TextEditorPrivate::showContextMenu()
{
    QMenu menu;
    QAction *action { nullptr };
    if (!q->isReadOnly()) {
        action = menu.addAction(tr("Undo"), q, &TextEditor::undo);
        action->setEnabled(q->isUndoAvailable());

        action = menu.addAction(tr("Redo"), q, &TextEditor::redo);
        action->setEnabled(q->isRedoAvailable());

        menu.addSeparator();

        action = menu.addAction(tr("Cut"), q, &TextEditor::cut);
        action->setEnabled(q->hasSelectedText());
    }

    action = menu.addAction(tr("Copy"), q, &TextEditor::copy);
    action->setEnabled(q->hasSelectedText());

    if (!q->isReadOnly()) {
        action = menu.addAction(tr("Paste"), q, &TextEditor::paste);
        action->setEnabled(q->SendScintilla(TextEditor::SCI_CANPASTE));

        action = menu.addAction(tr("Delete"), q, [this] { q->SendScintilla(TextEditor::SCI_CLEAR); });
        action->setEnabled(q->hasSelectedText());
    }

    menu.addSeparator();
    action = menu.addAction(tr("Select All"), q, [this] { q->selectAll(true); });
    action->setEnabled(q->length() != 0);

    // notify other plugin to add action.
    editor.contextMenu(QVariant::fromValue(&menu));
    menu.exec(QCursor::pos());
}

void TextEditorPrivate::showMarginMenu()
{
    QMenu menu;
    int line = 0, index = 0;
    q->getCursorPosition(&line, &index);

    if (q->hasBreakpoint(line)) {
        menu.addAction(tr("Remove Breakpoint"), q, [this, line] { editor.removeBreakpoint(fileName, line + 1); });
    } else {
        static QString text("Add a breakpoint on line %1");
        menu.addAction(text.arg(line + 1), q, [this, line] { editor.addBreakpoint(fileName, line + 1); });
    }

    // notify other plugin to add action.
    editor.marginMenu(QVariant::fromValue(&menu));
    menu.exec(QCursor::pos());
}

void TextEditorPrivate::gotoNextMark(uint mask)
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

void TextEditorPrivate::gotoPreviousMark(uint mask)
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

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor_p.h"
#include "utils/editorutils.h"
#include "utils/colordefine.h"
#include "lexer/lexermanager.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"

#include <Qsci/qsciapis.h>

#include <DGuiApplicationHelper>

#include <QScrollBar>
#include <QMenu>
#include <QDebug>
#include <QApplication>

static constexpr char DEFAULT_FONT_NAME[] { "Courier New" };

static constexpr int MARGIN_SYMBOLE_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_FOLDER_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_CHANGEBAR_DEFAULT_WIDTH = 3;

static constexpr int TAB_DEFAULT_WIDTH = 4;
static constexpr int NOTE_ANNOTATION_STYLE = 767;
static constexpr int WARNING_ANNOTATION_STYLE = 766;
static constexpr int ERROR_ANNOTATION_STYLE = 765;
static constexpr int FATAL_ANNOTATION_STYLE = 764;

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
    q->setAcceptDrops(false);
    q->setFrameShape(QFrame::NoFrame);
    q->SendScintilla(TextEditor::SCI_SETMOUSEDWELLTIME, 20);
    q->setAnnotationDisplay(TextEditor::AnnotationStandard);
    q->SendScintilla(TextEditor::SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR,
                     TextEditor::SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE);

    hoverTimer.setSingleShot(true);

    initMargins();
    updateColorTheme();
    updateSettings();
}

void TextEditorPrivate::initConnection()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TextEditorPrivate::onThemeTypeChanged);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSearch, this, &TextEditorPrivate::handleSearch);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqReplace, this, &TextEditorPrivate::handleReplace);

    connect(q, &TextEditor::SCN_ZOOM, q, &TextEditor::zoomValueChanged);
    connect(q, &TextEditor::SCN_DWELLSTART, this, &TextEditorPrivate::onDwellStart);
    connect(q, &TextEditor::SCN_DWELLEND, this, &TextEditorPrivate::onDwellEnd);
    connect(q, &TextEditor::SCN_MODIFIED, this, &TextEditorPrivate::onModified);
}

void TextEditorPrivate::initMargins()
{
    // Display line number
    q->setMarginLineNumbers(LineNumberMargin, true);
    setMarginVisible(LineNumberMargin, true);

    // folding
    q->setFolding(TextEditor::ArrowFoldStyle, FoldingMargin);

    // Set the margin symbol
    q->setMarginType(SymbolMargin, TextEditor::SymbolMargin);
    setMarginVisible(SymbolMargin, true);
    q->setMarginSensitivity(SymbolMargin, true);
    q->setMarginMarkerMask(SymbolMargin,
                           1 << Breakpoint | 1 << BreakpointDisabled
                                   | 1 << Bookmark | 1 << Runtime
                                   | 1 << RuntimeLineBackground | 1 << CustomLineBackground);

    // TODO: using picture to replace?
    q->markerDefine(TextEditor::Circle, Breakpoint);
    q->setMarkerForegroundColor(EditorColor::Table::get()->FireBrick, Breakpoint);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->FireBrick, Breakpoint);

    q->markerDefine(TextEditor::RightTriangle, Bookmark);
    q->setMarkerBackgroundColor(QColor(Qt::red), Bookmark);

    q->markerDefine(TextEditor::RightArrow, Runtime);
    q->setMarkerForegroundColor(EditorColor::Table::get()->YellowGreen, Runtime);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->YellowGreen, Runtime);

    q->markerDefine(TextEditor::Background, RuntimeLineBackground);
    q->setMarkerForegroundColor(EditorColor::Table::get()->YellowGreen, RuntimeLineBackground);
    q->setMarkerBackgroundColor(EditorColor::Table::get()->YellowGreen, RuntimeLineBackground);

    q->markerDefine(TextEditor::Background, CustomLineBackground);
}

void TextEditorPrivate::updateColorTheme()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        // editor
        q->setPaper(QColor("#2e2f30"));
        q->setCaretForegroundColor(QColor("#d6cf9a"));
        q->setCaretLineBackgroundColor(QColor("#373737"));

        // margins
        q->setFoldMarginColors(QColor("#252525"), QColor("#252525"));
        q->setMarginsBackgroundColor(QColor("#252525"));
        q->setMarginsForegroundColor(QColor("#bec0c2"));

        q->setMatchedBraceForegroundColor(QColor("#bec0c2"));
        q->setMatchedBraceBackgroundColor(QColor("#1d545c"));
    } else {
        // editor
        q->setPaper(QColor("#F8F8F8"));
        q->setCaretForegroundColor(QColor("#000000"));
        q->setCaretLineBackgroundColor(QColor("#c1ddee"));

        // margins
        q->setFoldMarginColors(QColor("#fefefe"), QColor("#fefefe"));
        q->setMarginsBackgroundColor(QColor("#fefefe"));
        q->setMarginsForegroundColor(QColor("#9f9f9f"));

        q->setMatchedBraceForegroundColor(QColor("#ff0000"));
        q->setMatchedBraceBackgroundColor(QColor("#b4eeb4"));
    }
}

void TextEditorPrivate::updateSettings()
{
    QFont font(DEFAULT_FONT_NAME, 10, QFont::Normal);
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

void TextEditorPrivate::loadLexer()
{
    if (fileName.isEmpty())
        return;

    using namespace support_file;
    auto id = Language::id(fileName);
    if (auto lexer = LexerManager::instance()->createSciLexer(id, fileName)) {
        lexer->setParent(q);
        q->setLexer(lexer);
        setMarginVisible(FoldingMargin, true);
    } else {
        setMarginVisible(FoldingMargin, false);
    }
}

void TextEditorPrivate::loadLSPStyle()
{
    if (!lspStyle) {
        lspStyle = new LSPStyle(q);
        lspStyle->initLspConnection();
    }

    lspStyle->updateTokens();
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
    menu.addAction(tr("Refactor"));
    menu.addSeparator();

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
    emit q->contextMenuRequested(&menu);
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

bool TextEditorPrivate::doFind(const QString &keyword, bool isForward)
{
    int line = 0, index = 0;
    q->getCursorPosition(&line, &index);
    // For forward search, 'index' needs to subtract the length of 'keyword',
    // otherwise it cannot jump to the previous one
    if (!isForward)
        index -= keyword.length();

    return q->findFirst(keyword, false, false, false, true, isForward, line, index);
}

QsciStyle TextEditorPrivate::createAnnotationStyle(int type)
{
    QFont font = q->font();
    font.setItalic(true);
    switch (type) {
    case AnnotationType::NoteAnnotation: {
        static QsciStyle style(NOTE_ANNOTATION_STYLE,
                               "Note",
                               EditorColor::Table::get()->Black,
                               EditorColor::Table::get()->Gainsboro,
                               font);
        return style;
    }
    case AnnotationType::ErrorAnnotation: {
        static QsciStyle style(ERROR_ANNOTATION_STYLE,
                               "Error",
                               EditorColor::Table::get()->FireBrick,
                               "#fbe8e8",
                               font);
        return style;
    }
    case AnnotationType::FatalAnnotation: {
        static QsciStyle style(FATAL_ANNOTATION_STYLE,
                               "Fatal",
                               EditorColor::Table::get()->FireBrick,
                               EditorColor::Table::get()->LightCoral,
                               font);
        return style;
    }
    case AnnotationType::WarningAnnotation:
        static QsciStyle style(WARNING_ANNOTATION_STYLE,
                               "Warning",
                               EditorColor::Table::get()->GoldenRod,
                               EditorColor::Table::get()->Ivory,
                               font);
        return style;
    }

    return {};
}

void TextEditorPrivate::adjustScrollBar()
{
    int currentRow = q->currentLineNumber();
    double ratio = static_cast<double>(currentRow) / q->lines();
    int scrollValue = static_cast<int>(ratio * q->verticalScrollBar()->maximum());
    q->verticalScrollBar()->setValue(scrollValue);
}

void TextEditorPrivate::onThemeTypeChanged()
{
    if (q->lexer()) {
        q->lexer()->resetStyle();
        q->setLexer(q->lexer());
    }

    if (lspStyle)
        lspStyle->refreshTokens();

    updateColorTheme();
}

void TextEditorPrivate::onDwellStart(int position, int x, int y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (position == -1)
        return;

    bool isKeyCtrl = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    if (isKeyCtrl)
        emit q->documentHoveredWithCtrl(position);
    else
        emit q->documentHovered(position);
}

void TextEditorPrivate::onDwellEnd(int position, int x, int y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (position == -1)
        return;

    emit q->documentHoverEnd(position);
}

void TextEditorPrivate::onModified(int pos, int mtype, const QString &text, int len, int added, int line,
                                   int foldNow, int foldPrev, int token, int annotationLinesAdded)
{
    Q_UNUSED(foldNow);
    Q_UNUSED(foldPrev);
    Q_UNUSED(token);
    Q_UNUSED(annotationLinesAdded);

    if (mtype & TextEditor::SC_MOD_INSERTTEXT) {
        emit q->textAdded(pos, len, added, text, line);
    } else if (mtype & TextEditor::SC_MOD_DELETETEXT) {
        emit q->textRemoved(pos, len, -added, text, line);
    }
}

void TextEditorPrivate::handleSearch(const QString &keyword, int operateType)
{
    switch (operateType) {
    case FindType::Previous:
        doFind(keyword, false);
        break;
    case FindType::Next:
        doFind(keyword, true);
        break;
    }
}

void TextEditorPrivate::handleReplace(const QString &srcText, const QString &destText, int operateType)
{
    switch (operateType) {
    case RepalceType::Repalce: {
        const auto &selectedText = q->selectedText();
        if (selectedText.compare(srcText, Qt::CaseInsensitive) == 0)
            q->replaceSelectedText(destText);
    } break;
    case RepalceType::FindAndReplace: {
        const auto &selectedText = q->selectedText();
        if (selectedText.compare(srcText, Qt::CaseInsensitive) == 0)
            q->replaceSelectedText(destText);
        doFind(srcText, true);
    } break;
    case RepalceType::RepalceAll: {
        while (doFind(srcText, true)) {
            q->replace(destText);
        }
    } break;
    }
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor_p.h"
#include "utils/editorutils.h"
#include "utils/colordefine.h"
#include "lexer/lexermanager.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"
#include "gui/settings/editorsettings.h"
#include "gui/settings/settingsdefine.h"
#include "services/option/optionutils.h"
#include "services/debugger/debuggerservice.h"

#include <Qsci/qsciapis.h>

#include <DGuiApplicationHelper>

#include <QToolTip>
#include <QScrollBar>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QSignalBlocker>

static constexpr char DEFAULT_FONT_NAME[] { "Noto Mono" };

static constexpr int MARGIN_SYMBOLE_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_FOLDER_DEFAULT_WIDTH = 14;
static constexpr int MARGIN_CHANGEBAR_DEFAULT_WIDTH = 3;

static constexpr int TAB_DEFAULT_WIDTH = 4;
static constexpr int NOTE_ANNOTATION_STYLE = 767;
static constexpr int WARNING_ANNOTATION_STYLE = 766;
static constexpr int ERROR_ANNOTATION_STYLE = 765;
static constexpr int FATAL_ANNOTATION_STYLE = 764;

DGUI_USE_NAMESPACE
using namespace dpfservice;

TextEditorPrivate::TextEditorPrivate(TextEditor *qq)
    : QObject(qq),
      q(qq),
      completionWidget(new CodeCompletionWidget(qq))
{
    init();
    initConnection();
}

void TextEditorPrivate::init()
{
    q->setFrameShape(QFrame::NoFrame);
    q->setAnnotationDisplay(TextEditor::AnnotationStandard);
    q->SendScintilla(TextEditor::SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR,
                     TextEditor::SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE);

    initMargins();
    updateColorTheme();
    updateSettings();
}

void TextEditorPrivate::initConnection()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TextEditorPrivate::resetThemeColor);
    connect(EditorSettings::instance(), &EditorSettings::valueChanged, this, &TextEditorPrivate::updateSettings);
    connect(qApp, &QApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) {
        if (state == Qt::ApplicationState::ApplicationInactive)
            q->cancelTips();
    });

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

    q->markerDefine(TextEditor::RightTriangle, Bookmark);
    q->setMarkerBackgroundColor(QColor(Qt::red), Bookmark);

    q->markerDefine(TextEditor::Background, RuntimeLineBackground);
    q->markerDefine(TextEditor::Background, CustomLineBackground);
}

void TextEditorPrivate::updateColorTheme()
{
    auto bpIcon = QIcon::fromTheme("breakpoint");
    q->markerDefine(bpIcon.pixmap(14, 14), Breakpoint);

    auto bpIconDis = QIcon::fromTheme("disabled_breakpoint");
    q->markerDefine(bpIconDis.pixmap(14, 14), BreakpointDisabled);

    auto rtIcon = QIcon::fromTheme("arrow");
    q->markerDefine(rtIcon.pixmap(14, 14), Runtime);

    q->setColor(q->palette().color(QPalette::WindowText));
    auto palette = QToolTip::palette();
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

        QColor rlbColor("#F2C55C");
        rlbColor.setAlpha(qRound(255 * 0.1));
        q->setMarkerForegroundColor(rlbColor, RuntimeLineBackground);
        q->setMarkerBackgroundColor(rlbColor, RuntimeLineBackground);

        // tooltip
        palette.setColor(QPalette::Inactive, QPalette::ToolTipText, Qt::lightGray);
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

        QColor rlbColor("#FFA715");
        rlbColor.setAlpha(qRound(255 * 0.1));
        q->setMarkerForegroundColor(rlbColor, RuntimeLineBackground);
        q->setMarkerBackgroundColor(rlbColor, RuntimeLineBackground);

        // tooltip
        palette.setColor(QPalette::Inactive, QPalette::ToolTipText, Qt::black);
    }

    QToolTip::setPalette(palette);
}

void TextEditorPrivate::updateSettings()
{
    fontName = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontFamily, DEFAULT_FONT_NAME).toString();
    fontSize = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontSize, 10).toInt();
    commentSettings = EditorSettings::instance()->getMap(Node::MimeTypeConfig);

    QFont font(fontName, fontSize, QFont::Normal);
    if (q->lexer())
        q->lexer()->setDefaultFont(font);
    else
        q->setFont(font);

    int fontZoom = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontZoom, 100).toInt();
    int realFontZoom = (fontZoom - 100) / 10;
    {
        // Avoid triggering the `zoomChanged` signal.
        QSignalBlocker blk(q);
        q->zoomTo(realFontZoom);
    }

    // Indentation
    auto tabPolicy = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::TabPolicy, 0).toInt();
    auto tabSize = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::TabSize, 4).toInt();
    auto autoIndent = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::EnableAutoIndentation, true).toBool();
    q->setIndentationsUseTabs(tabPolicy);
    q->setTabWidth(tabSize);
    q->setWhitespaceSize(3);
    q->setAutoIndent(autoIndent);

    int hoverTime = EditorSettings::instance()->value(Node::Behavior, Group::TipGroup, Key::TipActiveTime, 500).toInt();
    q->SendScintilla(TextEditor::SCI_SETMOUSEDWELLTIME, hoverTime);

    // Highlight the current line
    q->setCaretLineVisible(true);

    // Opening brace matching
    q->setBraceMatching(TextEditor::SloppyBraceMatch);

    q->setEolMode(TextEditor::EolUnix);
    q->setScrollWidth(1);
    q->setScrollWidthTracking(true);

    resetThemeColor();

    q->setMarginsFont(font);
    q->updateLineNumberWidth(false);
}

void TextEditorPrivate::loadLexer()
{
    if (fileName.isEmpty())
        return;

    using namespace support_file;
    auto id = Language::id(fileName);
    if (auto lexer = LexerManager::instance()->createSciLexer(id, fileName)) {
        lexer->setParent(q);
        QFont font(fontName, fontSize, QFont::Normal);
        lexer->setDefaultFont(font);
        q->setLexer(lexer);
        setMarginVisible(FoldingMargin, true);
    } else {
        setMarginVisible(FoldingMargin, false);
    }
}

void TextEditorPrivate::initLanguageClient()
{
    if (!languageClient)
        languageClient = new LanguageClientHandler(q);

    languageClient->updateTokens();
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
        menu.addAction(tr("Remove Breakpoint"), q, [this, line] { q->removeBreakpoint(line); });
        if (q->breakpointEnabled(line))
            menu.addAction(tr("Disable Breakpoint"), q, [this, line] { q->setBreakpointEnabled(line, false); });
        else
            menu.addAction(tr("Enable Breakpoint"), q, [this, line] { q->setBreakpointEnabled(line, true); });
        menu.addAction(tr("Add Condition"), q, [this, line] { q->setBreakpointCondition(line); });
    } else {
        static QString text(tr("Add a breakpoint on line %1"));
        menu.addAction(text.arg(line + 1), q, [this, line] { q->addBreakpoint(line); });
    }

    auto &ctx = dpfInstance.serviceContext();
    DebuggerService *debuggerService = ctx.service<DebuggerService>(DebuggerService::name());
    if (debuggerService->getDebugState() == AbstractDebugger::RunState::kStopped) {
        menu.addSeparator();
        menu.addAction(tr("jump to %1 line").arg(line + 1), q, [this, line] { editor.jumpToLine(fileName, line + 1); });
        menu.addAction(tr("run to %1 line").arg(line + 1), q, [this, line] { editor.runToLine(fileName, line + 1); });
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
    QScrollBar *scrollbar = q->verticalScrollBar();

    int totalLinesOfView = q->rect().height() / q->textHeight(0);
    auto cursorPoint = q->pointFromPosition(q->cursorPosition());

    int currentLine = cursorPoint.y() * totalLinesOfView / q->rect().height();
    int halfEditorLines = totalLinesOfView / 2;
    scrollbar->setValue(scrollbar->value() + currentLine - halfEditorLines);
}

QMap<int, int> TextEditorPrivate::allMarkers()
{
    QMap<int, int> markers;
    for (int line = 0; line < q->lines(); ++line) {
        int mask = static_cast<int>(q->markersAtLine(line));
        if (mask != 0)
            markers.insert(line, mask);
    }
    
    return markers;
}

void TextEditorPrivate::setMarkers(const QMap<int, int> &maskMap)
{
    int totalLine = q->lines();
    for (auto iter = maskMap.begin(); iter != maskMap.end(); ++iter) {
        if (iter.key() >= totalLine)
            break;
        
        if (iter.value() & (1 << Breakpoint)) {
            q->addBreakpoint(iter.key(), true);
        } else if (iter.value() & (1 << BreakpointDisabled)) {
            q->addBreakpoint(iter.key(), false);
        }
    }
}

void TextEditorPrivate::resetThemeColor()
{
    if (q->lexer()) {
        q->lexer()->resetStyle();
        q->setLexer(q->lexer());
    }

    if (languageClient)
        languageClient->refreshTokens();

    updateColorTheme();
}

void TextEditorPrivate::onDwellStart(int position, int x, int y)
{
    Q_UNUSED(position)

    int pos = q->positionFromPoint(x, y);
    if (pos == -1)
        return;

    emit q->documentHovered(pos);
}

void TextEditorPrivate::onDwellEnd(int position, int x, int y)
{
    Q_UNUSED(position)

    int pos = q->positionFromPoint(x, y);
    if (pos == -1)
        return;

    emit q->documentHoverEnd(pos);
}

void TextEditorPrivate::onModified(int pos, int mtype, const QString &text, int len, int added, int line,
                                   int foldNow, int foldPrev, int token, int annotationLinesAdded)
{
    Q_UNUSED(foldNow);
    Q_UNUSED(foldPrev);
    Q_UNUSED(token);
    Q_UNUSED(annotationLinesAdded);

    contentsChanged = true;
    if (isAutoCompletionEnabled && !text.isEmpty())
        editor.textChanged();
    
    if (added != 0) {
        int line = 0, index = 0;
        q->lineIndexFromPosition(pos, &line, &index);
        editor.lineChanged(fileName, line + 1, added);
    }

    if (mtype & TextEditor::SC_MOD_INSERTTEXT) {
        emit q->textAdded(pos, len, added, text, line);
    } else if (mtype & TextEditor::SC_MOD_DELETETEXT) {
        emit q->textRemoved(pos, len, -added, text, line);
    }
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor.h"
#include "private/texteditor_p.h"
#include "utils/editorutils.h"
#include "common/common.h"

#include "Qsci/qscistyledtext.h"

#include <QFile>
#include <QToolTip>
#include <QScrollBar>
#include <QFileDialog>
#include <QApplication>
#include <QContextMenuEvent>

TextEditor::TextEditor(QWidget *parent)
    : QsciScintilla(parent),
      d(new TextEditorPrivate(this))
{
    horizontalScrollBar()->setVisible(false);
    init();
}

TextEditor::~TextEditor()
{
}

void TextEditor::init()
{
    connect(this, &TextEditor::marginClicked, this, &TextEditor::onMarginClicked);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEditor::onScrollValueChanged);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::onCursorPositionChanged);
}

void TextEditor::setFile(const QString &fileName)
{
    if (d->fileName == fileName)
        return;

    beginUndoAction();
    d->isAutoCompletionEnabled = false;
    d->fileName = fileName;
    QString text;
    QFile file(d->fileName);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }

    setText(text.toUtf8());
    editor.fileOpened(fileName);
    d->loadLexer();
    d->loadLSPStyle();
    d->isAutoCompletionEnabled = true;
    endUndoAction();
}

QString TextEditor::getFile() const
{
    return d->fileName;
}

void TextEditor::save()
{
    if (!isModified())
        return;

    QFile file(d->fileName);
    if (!file.exists())
        return;

    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
        return;

    file.write(text().toUtf8());
    file.close();
    d->isSaved = true;
    emit fileSaved(d->fileName);
}

void TextEditor::saveAs()
{
    auto fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return;

    saveAs(fileName);
}

void TextEditor::saveAs(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate))
        return;

    d->fileName = fileName;
    file.write(text().toUtf8());
    file.close();
    d->isSaved = true;
}

void TextEditor::reload()
{
    int line = 0, index = 0;
    getCursorPosition(&line, &index);

    QString text;
    QFile file(d->fileName);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }
    setText(text.toUtf8());
    setModified(false);

    setCursorPosition(line, index);
}

void TextEditor::addBreakpoint(int line)
{
    if (hasBreakpoint(line))
        return;

    markerAdd(line, TextEditorPrivate::Breakpoint);
    editor.breakpointAdded(d->fileName, line + 1);
}

void TextEditor::removeBreakpoint(int line)
{
    markerDelete(line, TextEditorPrivate::Breakpoint);
    editor.breakpointRemoved(d->fileName, line + 1);
}

void TextEditor::toggleBreakpoint()
{
    int line = currentLineNumber();
    hasBreakpoint(line) ? removeBreakpoint(line) : addBreakpoint(line);
}

void TextEditor::setBreakpointEnabled(int line)
{
    Q_UNUSED(line)
    // TODO:
}

bool TextEditor::hasBreakpoint(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & (1 << TextEditorPrivate::Breakpoint));
}

void TextEditor::gotoNextBreakpoint()
{
    d->gotoNextMark(1 << TextEditorPrivate::Breakpoint);
}

void TextEditor::gotoPreviousBreakpoint()
{
    d->gotoPreviousMark(1 << TextEditorPrivate::Breakpoint);
}

void TextEditor::clearAllBreakpoints()
{
    markerDeleteAll(TextEditorPrivate::Breakpoint);
}

void TextEditor::setDebugLine(int line)
{
    removeDebugLine();

    gotoLine(line);
    markerAdd(line, TextEditorPrivate::Runtime);
    markerAdd(line, TextEditorPrivate::RuntimeLineBackground);
}

void TextEditor::removeDebugLine()
{
    markerDeleteAll(TextEditorPrivate::Runtime);
    markerDeleteAll(TextEditorPrivate::RuntimeLineBackground);
}

void TextEditor::addBookmark(int line)
{
    markerAdd(line, TextEditorPrivate::Bookmark);
}

void TextEditor::removeBookmark(int line)
{
    markerDelete(line, TextEditorPrivate::Bookmark);
}

bool TextEditor::hasBookmark(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & TextEditorPrivate::Bookmark);
}

void TextEditor::gotoNextBookmark()
{
    d->gotoNextMark(TextEditorPrivate::Bookmark);
}

void TextEditor::gotoPreviousBookmark()
{
    d->gotoPreviousMark(TextEditorPrivate::Bookmark);
}

void TextEditor::clearAllBookmarks()
{
    markerDeleteAll(TextEditorPrivate::Bookmark);
}

intptr_t TextEditor::searchInTarget(QByteArray &text2Find, size_t fromPos, size_t toPos) const
{
    SendScintilla(SCI_SETTARGETRANGE, fromPos, toPos);
    return SendScintilla(SCI_SEARCHINTARGET, text2Find.size(), reinterpret_cast<intptr_t>(text2Find.data()));
}

intptr_t TextEditor::replaceTarget(QByteArray &str2replace, intptr_t fromTargetPos, intptr_t toTargetPos) const
{
    if (fromTargetPos != -1 || toTargetPos != -1)
        SendScintilla(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);

    return SendScintilla(SCI_REPLACETARGET, str2replace.size(), reinterpret_cast<intptr_t>(str2replace.data()));
}

int TextEditor::currentLineNumber()
{
    auto num = SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS));
    return static_cast<int>(num);
}

void TextEditor::gotoLine(int line)
{
    ensureLineVisible(line);

    SendScintilla(SCI_GOTOLINE, line);
    setFocus();
    d->adjustScrollBar();
}

void TextEditor::gotoPosition(int pos)
{
    SendScintilla(SCI_GOTOPOS, pos);
    ensureCursorVisible();
    setFocus();
    d->adjustScrollBar();
}

int TextEditor::cursorLastPosition()
{
    return d->lastCursorPos;
}

int TextEditor::cursorPosition()
{
    return static_cast<int>(SendScintilla(TextEditor::SCI_GETCURRENTPOS));
}

void TextEditor::setLineBackgroundColor(int line, const QColor &color)
{
    markerAdd(line, TextEditorPrivate::CustomLineBackground);
    setMarkerBackgroundColor(color, TextEditorPrivate::CustomLineBackground);
}

void TextEditor::resetLineBackgroundColor(int line)
{
    markerDelete(line, TextEditorPrivate::CustomLineBackground);
}

void TextEditor::clearLineBackgroundColor()
{
    markerDeleteAll(TextEditorPrivate::CustomLineBackground);
}

void TextEditor::showTips(const QString &tips)
{
    int pos = d->cursorPosition();
    showTips(pos, tips);
}

void TextEditor::showTips(int pos, const QString &tips)
{
    if (!hasFocus())
        return;

    auto point = pointFromPosition(pos);
    QToolTip::showText(mapToGlobal(point), tips, this);
}

void TextEditor::cancelTips()
{
    QToolTip::hideText();
}

void TextEditor::addAnnotation(const QString &title, const QString &content, int line, int type)
{
    QString typeStr;
    switch (type) {
    case AnnotationType::NoteAnnotation:
        typeStr = "Note";
        break;
    case AnnotationType::ErrorAnnotation:
        typeStr = "Error";
        break;
    case AnnotationType::FatalAnnotation:
        typeStr = "Fatal";
        break;
    case AnnotationType::WarningAnnotation:
        typeStr = "Warning";
        break;
    }

    d->annotationRecords.insertMulti(title, line);
    static QString formatText("%1:\n%2:\n%3");
    auto msg = formatText.arg(title, typeStr, content);
    addAnnotation(msg, line, type);
}

void TextEditor::addAnnotation(const QString &content, int line, int type)
{
    auto style = d->createAnnotationStyle(type);
    annotate(line, content, style);
}

void TextEditor::removeAnnotation(const QString &title)
{
    if (!d->annotationRecords.contains(title))
        return;

    auto lineList = d->annotationRecords.values(title);
    d->annotationRecords.remove(title);

    for (int line : lineList)
        clearAnnotations(line);
}

QPoint TextEditor::pointFromPosition(int position)
{
    int x = static_cast<int>(SendScintilla(SCI_POINTXFROMPOSITION, 0, position));
    int y = static_cast<int>(SendScintilla(SCI_POINTYFROMPOSITION, 0, position));

    return QPoint(x, y);
}

int TextEditor::positionFromPoint(int x, int y)
{
    int pos = static_cast<int>(SendScintilla(SCI_POSITIONFROMPOINT, static_cast<ulong>(x), y));
    return pos;
}

void TextEditor::replaceRange(int lineFrom, int indexFrom, int lineTo, int indexTo, const QString &text)
{
    SendScintilla(SCI_CLEARSELECTIONS);

    setSelection(lineFrom, indexFrom, lineTo, indexTo);
    replaceSelectedText(text);
}

void TextEditor::replaceRange(int startPosition, int endPosition, const QString &text)
{
    int startLine = 0, startIndex = 0;
    int endLine = 0, endIndex = 0;

    lineIndexFromPosition(startPosition, &startLine, &startIndex);
    lineIndexFromPosition(endPosition, &endLine, &endIndex);

    replaceRange(startLine, startIndex, endLine, endIndex, text);
}

void TextEditor::insertText(const QString &text)
{
    auto textData = text.toLocal8Bit();

    SendScintilla(SCI_INSERTTEXT, static_cast<ulong>(d->cursorPosition()), textData.constData());
    SendScintilla(SCI_SETEMPTYSELECTION, d->cursorPosition() + textData.size());
}

bool TextEditor::isSaved() const
{
    return d->isSaved;
}

void TextEditor::resetSaveState()
{
    d->isSaved = false;
}

LSPStyle *TextEditor::lspStyle() const
{
    return d->lspStyle;
}

int TextEditor::wordStartPositoin(int position)
{
    return static_cast<int>(SendScintilla(SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true));
}

int TextEditor::wordEndPosition(int position)
{
    return static_cast<int>(SendScintilla(SCI_WORDENDPOSITION, static_cast<ulong>(position), true));
}

void TextEditor::switchHeaderSource()
{
    if (!d->lspStyle)
        return;

    d->lspStyle->switchHeaderSource(d->fileName);
}

void TextEditor::followSymbolUnderCursor()
{
    if (!d->lspStyle)
        return;

    d->lspStyle->followSymbolUnderCursor();
}

void TextEditor::findUsage()
{
    if (!d->lspStyle)
        return;

    d->lspStyle->findUsagesActionTriggered();
}

void TextEditor::renameSymbol()
{
    if (!d->lspStyle)
        return;

    d->lspStyle->renameActionTriggered();
}

void TextEditor::setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key)
{
    if (!d->completionWidget)
        return;

    d->completionWidget->setCompletion(info, icon, key);
}

QString TextEditor::cursorBeforeText() const
{
    int pos = d->cursorPosition();
    return text(0, pos);
}

QString TextEditor::cursorBehindText() const
{
    int pos = d->cursorPosition();
    return text(pos, length());
}

void TextEditor::setAutomaticInvocationEnabled(bool enabled)
{
    d->isAutoCompletionEnabled = enabled;
}

bool TextEditor::isAutomaticInvocationEnabled() const
{
    return d->isAutoCompletionEnabled;
}

void TextEditor::onMarginClicked(int margin, int line, Qt::KeyboardModifiers state)
{
    Q_UNUSED(state)

    if (margin == TextEditorPrivate::SymbolMargin) {
        // The value of `state` is not accurate
        auto modifers = QApplication::queryKeyboardModifiers();
        int mask = static_cast<int>(markersAtLine(line));
        switch (modifers) {
        case Qt::ControlModifier:
            if (mask & (1 << TextEditorPrivate::Bookmark))
                removeBookmark(line);
            else
                addBookmark(line);
            break;
        default:
            if (mask & (1 << TextEditorPrivate::Breakpoint))
                editor.removeBreakpoint(d->fileName, line + 1);
            else
                editor.addBreakpoint(d->fileName, line + 1);
        }
    }
}

void TextEditor::updateLineNumberWidth(bool isDynamicWidth)
{
    auto linesVisible = SendScintilla(SCI_LINESONSCREEN);
    if (!linesVisible)
        return;

    int nbDigits = 0;
    if (isDynamicWidth) {
        auto firstVisibleLineVis = SendScintilla(SCI_GETFIRSTVISIBLELINE);
        auto lastVisibleLineVis = linesVisible + firstVisibleLineVis + 1;
        auto lastVisibleLineDoc = SendScintilla(SCI_DOCLINEFROMVISIBLE, lastVisibleLineVis);

        nbDigits = EditorUtils::nbDigitsFromNbLines(lastVisibleLineDoc);
        nbDigits = nbDigits < 4 ? 4 : nbDigits;
    } else {
        auto nbLines = SendScintilla(SCI_GETLINECOUNT);
        nbDigits = EditorUtils::nbDigitsFromNbLines(nbLines);
        nbDigits = nbDigits < 4 ? 4 : nbDigits;
    }

    auto pixelWidth = 6 + nbDigits * SendScintilla(SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<intptr_t>("8"));
    setMarginWidth(TextEditorPrivate::LineNumberMargin, static_cast<int>(pixelWidth));
}

void TextEditor::onScrollValueChanged(int value)
{
    // Adjust the line width every 200 units of scroll
    if (qAbs(value - d->preFirstLineNum) > 200) {
        d->preFirstLineNum = value;
        updateLineNumberWidth(true);
    }
}

void TextEditor::onCursorPositionChanged(int line, int index)
{
    Q_UNUSED(line)

    int pos = positionFromLineIndex(line, index);
    d->lastCursorPos = pos;
}

void TextEditor::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
    QsciScintilla::focusOutEvent(event);
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
    if (d->completionWidget->processKeyPressEvent(event))
        return;

    QsciScintilla::keyPressEvent(event);
}

void TextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    if (!contextMenuNeeded(event->pos().x(), event->pos().y()))
        return;

    int xPos = event->pos().x();
    if (xPos <= d->marginsWidth()) {
        d->showMarginMenu();
    } else {
        d->showContextMenu();
    }
}

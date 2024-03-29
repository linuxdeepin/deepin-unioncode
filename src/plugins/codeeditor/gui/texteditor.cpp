// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor.h"
#include "private/texteditor_p.h"
#include "utils/editorutils.h"
#include "common/common.h"

#include "Qsci/qscistyledtext.h"

#include <QFile>
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
    emit fileSaved(d->fileName);
}

void TextEditor::saveAs()
{
    d->fileName = QFileDialog::getSaveFileName(this);
    if (d->fileName.isEmpty())
        return;

    QFile file(d->fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate))
        return;

    file.write(text().toUtf8());
    file.close();
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

int TextEditor::cursorPosition()
{
    return d->lastCursorPos;
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

    auto data = tips.toLocal8Bit();
    SendScintilla(SCI_CALLTIPSHOW, static_cast<uintptr_t>(pos), data.data());
}

void TextEditor::cancelTips()
{
    SendScintilla(SCI_CALLTIPCANCEL);
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

void TextEditor::replaceRange(int lineFrom, int indexFrom, int lineTo, int indexTo, const QString &text)
{
    SendScintilla(SCI_CLEARSELECTIONS);

    setSelection(lineFrom, indexFrom, lineTo, indexTo);
    replaceSelectedText(text);
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
    if (d->lastCursorPos != pos)
        d->lastCursorPos = pos;
}

void TextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    cancelTips();
    if (!contextMenuNeeded(event->pos().x(), event->pos().y()))
        return;

    int xPos = event->pos().x();
    // Delay the display of the menu to ensure that tips can disappear normally
    QTimer::singleShot(20, this, [this, xPos] {
        if (xPos <= d->marginsWidth()) {
            d->showMarginMenu();
        } else {
            d->showContextMenu();
        }
    });
}

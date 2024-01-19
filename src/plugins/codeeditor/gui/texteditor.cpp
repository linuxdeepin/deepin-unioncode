// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor.h"
#include "private/texteditor_p.h"
#include "utils/editorutils.h"

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
    d->loadDefaultLexer();
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
    markerAdd(line, TextEditorPrivate::BreakpointSymbol);
}

void TextEditor::removeBreakpoint(int line)
{
    markerDelete(line, TextEditorPrivate::BreakpointSymbol);
}

void TextEditor::setBreakpointEnabled(int line)
{
    Q_UNUSED(line)
    // TODO:
}

bool TextEditor::hasBreakpoint(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & TextEditorPrivate::BreakpointMask);
}

void TextEditor::gotoNextBreakpoint()
{
    d->gotoNextMark(TextEditorPrivate::BreakpointMask);
}

void TextEditor::gotoPreviousBreakpoint()
{
    d->gotoPreviousMark(TextEditorPrivate::BreakpointMask);
}

void TextEditor::clearAllBreakpoints()
{
    markerDeleteAll(TextEditorPrivate::BreakpointSymbol);
}

void TextEditor::addBookmark(int line)
{
    markerAdd(line, TextEditorPrivate::BookmarkSymbol);
}

void TextEditor::removeBookmark(int line)
{
    markerDelete(line, TextEditorPrivate::BookmarkSymbol);
}

bool TextEditor::hasBookmark(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & TextEditorPrivate::BookmarkMask);
}

void TextEditor::gotoNextBookmark()
{
    d->gotoNextMark(TextEditorPrivate::BookmarkMask);
}

void TextEditor::gotoPreviousBookmark()
{
    d->gotoPreviousMark(TextEditorPrivate::BookmarkMask);
}

void TextEditor::clearAllBookmarks()
{
    markerDeleteAll(TextEditorPrivate::BookmarkSymbol);
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
}

void TextEditor::gotoPosition(int pos)
{
    SendScintilla(SCI_GOTOPOS, pos);
    ensureCursorVisible();
}

int TextEditor::cursorPosition()
{
    return d->lastCursorPos;
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
        auto modifers = QApplication::queryKeyboardModifiers();
        int mask = static_cast<int>(markersAtLine(line));
        switch (modifers) {
        case Qt::ControlModifier:   // 按下Ctrl键
            if (mask & TextEditorPrivate::BookmarkMask)
                markerDelete(line, TextEditorPrivate::BookmarkSymbol);
            else
                markerAdd(line, TextEditorPrivate::BookmarkSymbol);   // 添加书签
            break;
        default:
            if (mask & TextEditorPrivate::BreakpointMask)
                markerDelete(line, TextEditorPrivate::BreakpointSymbol);
            else
                markerAdd(line, TextEditorPrivate::BreakpointSymbol);   // 添加断点
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
    if (!contextMenuNeeded(event->pos().x(), event->pos().y()))
        return;

    if (event->pos().x() <= d->marginsWidth()) {
        d->showMarginMenu();
    } else {
        d->showContextMenu();
    }
}

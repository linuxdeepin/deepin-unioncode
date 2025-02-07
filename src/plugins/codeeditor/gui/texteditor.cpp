// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor.h"
#include "private/texteditor_p.h"
#include "utils/editorutils.h"
#include "encodes/detectcode.h"
#include "common/common.h"
#include "common/tooltip/tooltip.h"
#include "settings/settingsdefine.h"
#include "utils/resourcemanager.h"

#include "services/editor/editor_define.h"

#include "Qsci/qscidocument.h"
#include "Qsci/qscilexer.h"

#include <DDialog>

#include <QFile>
#include <QScrollBar>
#include <QFileDialog>
#include <QApplication>
#include <QContextMenuEvent>

DWIDGET_USE_NAMESPACE

TextEditor::TextEditor(QWidget *parent)
    : QsciScintilla(parent)
{
    d = new TextEditorPrivate(this);
    horizontalScrollBar()->setVisible(false);
    init();
}

TextEditor::~TextEditor()
{
    delete d;
}

void TextEditor::init()
{
    connect(this, &TextEditor::marginClicked, this, &TextEditor::onMarginClicked);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEditor::onScrollValueChanged);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::onCursorPositionChanged);
}

void TextEditor::openFile(const QString &fileName)
{
    if (d->fileName == fileName)
        return;

    beginUndoAction();
    d->isAutoCompletionEnabled = false;
    d->fileName = fileName;
    setReadOnly(!QFileInfo(fileName).isWritable());
    d->readFile("");
    setModified(false);
    editor.fileOpened(fileName);
    d->loadLexer();
    d->initLanguageClient();
    d->isAutoCompletionEnabled = true;
    endUndoAction();
}

void TextEditor::openFileWithDocument(const QString &fileName, const QsciDocument &doc)
{
    if (d->fileName == fileName)
        return;

    d->fileName = fileName;
    setDocument(doc);
    d->loadLexer();
    d->initLanguageClient();
    d->isAutoCompletionEnabled = true;

    if (isModified())
        emit modificationChanged(true);
}

QString TextEditor::getFile() const
{
    return d->fileName;
}

QString TextEditor::documentEncode() const
{
    return d->documentEncode;
}

void TextEditor::save()
{
    if (!isModified())
        return;

    QFile file(d->fileName);
    if (!file.exists())
        return;

    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        DDialog dialog;
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.setWindowTitle(tr("Save File"));
        QString msg(tr("The file \"%1\" has no write permission. Please add write permission and try again"));
        dialog.setMessage(msg.arg(d->fileName));
        dialog.addButton(tr("Ok", "button"), true, DDialog::ButtonRecommend);
        dialog.exec();
        return;
    }

    QByteArray fileContent = text().toLocal8Bit();
    if (!fileContent.isEmpty()) {
        QByteArray Outdata;
        DetectCode::changeFileEncodingFormat(fileContent, Outdata, QString("UTF-8"), d->documentEncode);
        if (Outdata.isEmpty()) {
            qWarning() << qPrintable(QString("iconv Encode Transformat from '%1' to '%2' Fail! start QTextCodec Encode Transformat.")
                                             .arg(QString("UTF-8"), d->documentEncode));
            // Using QTextCodec to convert
            QTextCodec *codec = QTextCodec::codecForName(d->documentEncode.toUtf8());
            if (codec) {
                QByteArray encodedString = codec->fromUnicode(fileContent);
                if (encodedString.isEmpty()) {
                    qWarning() << qPrintable("Both iconv and QTextCodec Encode Transformat Fail!");
                } else {
                    qWarning() << qPrintable(QString("QTextCodec Encode Transformat from '%1' to '%2' Success!")
                                                     .arg(QString("UTF-8"), d->documentEncode));
                    Outdata = encodedString;
                }
            } else {
                qWarning() << qPrintable("Unsupported QTextCodec format:") << d->documentEncode;
            }
        }

        if (!Outdata.isEmpty())
            file.write(Outdata);
    } else {
        file.write(fileContent);
    }

    file.close();
    setModified(false);
    editor.fileSaved(d->fileName);
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
    setModified(false);
}

void TextEditor::reload()
{
    int line = 0, index = 0;
    getCursorPosition(&line, &index);
    const auto &markers = d->allMarkers();
    d->readFile("");
    d->setMarkers(markers);
    setCursorPosition(line, index);
    emit textChanged();
}

bool TextEditor::reload(const QString &encode)
{
    if (encode == d->documentEncode)
        return false;

    if (length() == 0) {
        d->documentEncode = encode;
        return true;
    }

    if (isModified()) {
        DDialog dlg(tr("Encoding changed. Do you want to save the file now?"), "", this);
        dlg.setIcon(QIcon::fromTheme("ide"));
        dlg.addButton(QString(tr("Cancel", "button")));
        dlg.addButton(QString(tr("Save", "button")), true, DDialog::ButtonRecommend);
        int res = dlg.exec();
        if (res == 0)
            return false;

        if (res == 1)
            save();
    }
    return d->readFile(encode);
}

void TextEditor::addBreakpoint(int line, bool enabled)
{
    if (hasBreakpoint(line))
        return;

    if (enabled) {
        markerAdd(line, TextEditorPrivate::Breakpoint);
    } else {
        markerAdd(line, TextEditorPrivate::BreakpointDisabled);
    }

    editor.breakpointAdded(d->fileName, line, enabled);
}

void TextEditor::removeBreakpoint(int line)
{
    if (breakpointEnabled(line))
        markerDelete(line, TextEditorPrivate::Breakpoint);
    else
        markerDelete(line, TextEditorPrivate::BreakpointDisabled);

    editor.breakpointRemoved(d->fileName, line);
}

void TextEditor::toggleBreakpoint()
{
    int line = currentLineNumber();
    hasBreakpoint(line) ? removeBreakpoint(line) : addBreakpoint(line);
}

void TextEditor::setBreakpointEnabled(int line, bool enabled)
{
    if (!hasBreakpoint(line))
        return;

    bool hasEnabled = breakpointEnabled(line);
    if (hasEnabled == enabled)
        return;

    if (enabled) {
        markerDelete(line, TextEditorPrivate::BreakpointDisabled);
        markerAdd(line, TextEditorPrivate::Breakpoint);
    } else {
        markerDelete(line, TextEditorPrivate::Breakpoint);
        markerAdd(line, TextEditorPrivate::BreakpointDisabled);
    }
    editor.breakpointStatusChanged(d->fileName, line, enabled);
}

void TextEditor::setBreakpointCondition(int line)
{
    editor.setBreakpointCondition(d->fileName, line);
}

bool TextEditor::breakpointEnabled(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & (1 << TextEditorPrivate::Breakpoint));
}

bool TextEditor::hasBreakpoint(int line)
{
    int mask = static_cast<int>(markersAtLine(line));
    return (mask & (1 << TextEditorPrivate::Breakpoint) || mask & (1 << TextEditorPrivate::BreakpointDisabled));
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
    d->lastCursorNeedRecord = false;
    d->postionChangedByGoto = true;
}

void TextEditor::gotoPosition(int pos)
{
    SendScintilla(SCI_GOTOPOS, pos);
    ensureCursorVisible();
    setFocus();
    d->adjustScrollBar();
    d->lastCursorNeedRecord = false;
    d->postionChangedByGoto = true;
}

int TextEditor::cursorLastPosition()
{
    return d->lastCursorPos;
}

int TextEditor::cursorPosition()
{
    return static_cast<int>(SendScintilla(TextEditor::SCI_GETCURRENTPOS));
}

int TextEditor::backgroundMarkerDefine(const QColor &color, int defaultMarker)
{
    int marker = markerDefine(Background, defaultMarker);
    setMarkerBackgroundColor(color, marker);
    return marker;
}

void TextEditor::setRangeBackgroundColor(int startLine, int endLine, int marker)
{
    startLine = qMax(startLine, 0);
    endLine = qMin(endLine, lines() - 1);
    if (startLine > endLine)
        return;

    d->markerCache.insert(marker, { startLine, endLine });
    for (; startLine <= endLine; ++startLine) {
        markerAdd(startLine, marker);
    }
}

void TextEditor::getBackgroundRange(int marker, int *startLine, int *endLine)
{
    if (!d->markerCache.contains(marker))
        return;

    if (!startLine || !endLine)
        return;

    *startLine = d->markerCache[marker].startLine;
    *endLine = d->markerCache[marker].endLine;
}

void TextEditor::clearAllBackgroundColor(int marker)
{
    d->markerCache.remove(marker);
    markerDeleteAll(marker);
}

void TextEditor::showTips(const QString &tips)
{
    int pos = d->cursorPosition();
    showTips(pos, tips);
}

void TextEditor::showTips(int pos, const QString &tips)
{
    if (!hasFocus() || !d->tipsDisplayable)
        return;

    bool isCtrlPressed = qApp->queryKeyboardModifiers().testFlag(Qt::ControlModifier);
    if (isCtrlPressed)
        return;

    auto point = pointFromPosition(pos);
    ToolTip::show(mapToGlobal(point), tips, this);
}

void TextEditor::showTips(int pos, QWidget *w)
{
    if (!hasFocus() || !d->tipsDisplayable)
        return;

    bool isCtrlPressed = qApp->queryKeyboardModifiers().testFlag(Qt::ControlModifier);
    if (isCtrlPressed)
        return;

    auto point = pointFromPosition(pos);
    ToolTip::show(mapToGlobal(point), w, this);
}

void TextEditor::cancelTips()
{
    ToolTip::hideImmediately();
}

void TextEditor::addAnnotation(const QString &title, const QString &content, int line, int type)
{
    QString typeStr;
    switch (type) {
    case dpfservice::Edit::TipAnnotation:
        typeStr = "Tip";
        break;
    case dpfservice::Edit::NoteAnnotation:
        typeStr = "Note";
        break;
    case dpfservice::Edit::ErrorAnnotation:
        typeStr = "Error";
        break;
    case dpfservice::Edit::FatalAnnotation:
        typeStr = "Fatal";
        break;
    case dpfservice::Edit::WarningAnnotation:
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

void TextEditor::addEOLAnnotation(const QString &title, const QString &content, int line, int type)
{
    d->eOLAnnotationRecords.insertMulti(title, line);
    auto style = d->createAnnotationStyle(type);
    eOLAnnotate(line, content, style);
}

void TextEditor::removeEOLAnnotation(const QString &title)
{
    if (!d->eOLAnnotationRecords.contains(title))
        return;

    auto lineList = d->eOLAnnotationRecords.values(title);
    d->eOLAnnotationRecords.remove(title);

    for (int line : lineList)
        clearEOLAnnotations(line);
}

void TextEditor::commentOperation()
{
    QStringList fileCommentSettings = getFileCommentSettings(d->commentSettings);
    if (fileCommentSettings.isEmpty())
        return;

    int lineFrom = 0, indexFrom = 0;
    int lineTo = 0, indexTo = 0;

    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    if (!hasSelectedText()) {
        int cursorLine = 0, cursorIndex = 0;
        getCursorPosition(&cursorLine, &cursorIndex);
        // The `lineLength` interface gets the byte length
        // and needs to get the character length
        const auto &lineText = text(cursorLine);
        int lineLastIndex = lineText.length();
        if (lineText.endsWith('\n'))
            lineLastIndex--;

        if (hasUncommentedLines(cursorLine, cursorLine, 0, lineLastIndex, fileCommentSettings))
            addCommentToSelectedLines(cursorLine, cursorLine, 0, lineLastIndex, fileCommentSettings);
        else
            delCommentToSelectedLines(cursorLine, cursorLine, 0, lineLastIndex, fileCommentSettings);
        return;
    }

    if (hasUncommentedLines(lineFrom, lineTo, indexFrom, indexTo, fileCommentSettings))
        addCommentToSelectedLines(lineFrom, lineTo, indexFrom, indexTo, fileCommentSettings);
    else
        delCommentToSelectedLines(lineFrom, lineTo, indexFrom, indexTo, fileCommentSettings);
}

QString TextEditor::getFileType()
{
    // Get mimetype using only the file name
    QMimeDatabase mimeDatabase;
    QString mimeTypeName = mimeDatabase.mimeTypeForFile(d->fileName, QMimeDatabase::MatchExtension).name();
    return mimeTypeName;
}

QStringList TextEditor::getFileCommentSettings(const QMap<QString, QVariant> &commentSettings)
{
    for (int i = 0; i < commentSettings.count(); i++) {
        if (commentSettings[QString::number(i)].toMap().value(Key::MimeType).toString().split(";").contains(getFileType())) {
            return QStringList() << commentSettings[QString::number(i)].toMap().value(Key::SingleLineComment).toString()
                                 << commentSettings[QString::number(i)].toMap().value(Key::StartMultiLineComment).toString()
                                 << commentSettings[QString::number(i)].toMap().value(Key::EndMultiLineComment).toString();
        }
    }
    return QStringList();
}

bool TextEditor::hasUncommentedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings)
{
    if (selectionStatus(lineFrom, lineTo, indexFrom, indexTo)) {
        if (selectedText().startsWith(settings.at(CommentSettings::BlockStart)) && selectedText().endsWith(settings.at(CommentSettings::BlockEnd)))
            return false;
        return true;
    }
    setSelection(lineFrom, indexFrom, lineTo, indexTo);
    QStringList lines = selectedText().split(QRegExp("\\r\\n|\\n|\\r"));
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty())
            continue;
        QRegularExpression regex;
        regex.setPattern("^\\s*" + settings.at(CommentSettings::Line));
        QRegularExpressionMatch match = regex.match(line);
        if (!match.hasMatch()) {
            return true;
        }
    }
    return false;
}

void TextEditor::addCommentToSelectedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings)
{
    QString selectedTexts = this->selectedText();
    if (selectionStatus(lineFrom, lineTo, indexFrom, indexTo)) {
        selectedTexts.insert(0, settings.at(CommentSettings::BlockStart));
        selectedTexts.append(settings.at(CommentSettings::BlockEnd));
        this->replaceRange(lineFrom, indexFrom, lineTo, indexTo, selectedTexts);
        if (lineFrom == lineTo)
            setSelection(lineFrom, indexFrom, lineTo, indexTo + settings.at(CommentSettings::BlockEnd).size() + settings.at(1).size());
        else
            setSelection(lineFrom, indexFrom, lineTo, indexTo + settings.at(CommentSettings::BlockEnd).size());
    } else {
        setSelection(lineFrom, 0, lineTo, indexTo);
        selectedTexts = this->selectedText();
        selectedTexts = addCommentPrefix(selectedTexts, settings.at(CommentSettings::Line));
        this->replaceRange(lineFrom, 0, lineTo, indexTo, selectedTexts);
        if (text(lineTo).trimmed().isEmpty() || indexTo == 0)
            setSelection(lineFrom, indexFrom, lineTo, indexTo);
        else
            setSelection(lineFrom, indexFrom, lineTo, indexTo + settings.at(CommentSettings::Line).size());
    }
}

QString TextEditor::addCommentPrefix(const QString &selectedTexts, const QString &commentSymbol)
{
    QStringList lines = selectedTexts.split(QRegExp("\\r\\n|\\n|\\r"));
    QStringList prefixedLines;

    for (const QString &line : lines) {
        if (!line.trimmed().isEmpty()) {
            QString prefixedLine = commentSymbol + line;
            prefixedLines.append(prefixedLine);
        } else {
            prefixedLines.append(line);
        }
    }
    QString result = prefixedLines.join("\n");
    return result;
}

QString TextEditor::delCommentPrefix(const QString &selectedTexts, const QString &commentSymbol)
{
    QStringList lines = selectedTexts.split(QRegExp("\\r\\n|\\n|\\r"));
    QStringList prefixedLines;
    QRegularExpression regex(commentSymbol);

    for (const QString &line : lines) {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            QString delPrefixLine = line.mid(0, match.capturedStart()) + line.mid(match.capturedEnd());
            prefixedLines.append(delPrefixLine);
        } else {
            prefixedLines.append(line);
        }
    }
    QString result = prefixedLines.join("\n");
    return result;
}

void TextEditor::delCommentToSelectedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings)
{
    QString selectedTexts = this->selectedText();
    if (selectionStatus(lineFrom, lineTo, indexFrom, indexTo)) {
        selectedTexts.replace(settings.at(1), "");
        selectedTexts.replace(settings.at(2), "");
        this->replaceRange(lineFrom, indexFrom, lineTo, indexTo, selectedTexts);
        if (lineFrom == lineTo)
            setSelection(lineFrom, indexFrom, lineTo, indexTo - settings.at(CommentSettings::BlockEnd).size() - settings.at(1).size());
        else
            setSelection(lineFrom, indexFrom, lineTo, indexTo - settings.at(CommentSettings::BlockEnd).size());
    } else {
        setSelection(lineFrom, 0, lineTo, indexTo);
        selectedTexts = delCommentPrefix(this->selectedText(), settings.at(CommentSettings::Line));
        this->replaceRange(lineFrom, 0, lineTo, indexTo, selectedTexts);
        setSelection(lineFrom, indexFrom, lineTo, indexTo - settings.at(CommentSettings::Line).size());
    }
}

bool TextEditor::selectionStatus(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo)
{
    QString startLineText = this->text(lineFrom);
    QString endLineText = this->text(lineTo);
    bool hasNonSpaceBeforeStart = !startLineText.left(indexFrom).trimmed().isEmpty();
    bool hasNonSpaceAfterEnd = !endLineText.mid(indexTo).trimmed().isEmpty();
    bool res = hasNonSpaceBeforeStart || (hasNonSpaceAfterEnd && (indexTo != 0));
    return res;
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

void TextEditor::replaceRange(int lineFrom, int indexFrom, int lineTo, int indexTo, const QString &text, bool changePos)
{
    int startPos = positionFromLineIndex(lineFrom, indexFrom);
    int endPos = positionFromLineIndex(lineTo, indexTo);
    replaceRange(startPos, endPos, text, changePos);
}

void TextEditor::replaceRange(int startPosition, int endPosition, const QString &text, bool changePos)
{
    d->isAutoCompletionEnabled = false;
    SendScintilla(SCI_CLEARSELECTIONS);
    SendScintilla(SCI_SETTARGETSTART, startPosition);
    SendScintilla(SCI_SETTARGETEND, endPosition);
    SendScintilla(SCI_REPLACETARGET, -1, textAsBytes(text).constData());
    if (changePos)
        SendScintilla(SCI_GOTOPOS, startPosition + text.length());
    d->isAutoCompletionEnabled = true;
}

void TextEditor::insertText(const QString &text)
{
    auto textData = text.toLocal8Bit();

    SendScintilla(SCI_INSERTTEXT, static_cast<ulong>(d->cursorPosition()), textData.constData());
    SendScintilla(SCI_SETEMPTYSELECTION, d->cursorPosition() + textData.size());
}

LanguageClientHandler *TextEditor::languageClient() const
{
    return d->languageClient;
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
    if (!d->languageClient)
        return;

    d->languageClient->switchHeaderSource(d->fileName);
}

void TextEditor::followSymbolUnderCursor()
{
    if (!d->languageClient)
        return;

    d->languageClient->followSymbolUnderCursor();
}

void TextEditor::findUsage()
{
    if (!d->languageClient)
        return;

    d->languageClient->findUsagesActionTriggered();
}

void TextEditor::renameSymbol()
{
    if (!d->languageClient)
        return;

    d->languageClient->renameActionTriggered();
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

bool TextEditor::showLineWidget(int line, QWidget *widget)
{
    if (line == 0)
        line += 1;

    if (line < 0 || line >= lines())
        return false;

    if (d->lineWidgetContainer->isVisible())
        closeLineWidget();

    d->showAtLine = line;
    d->setContainerWidget(widget);
    return true;
}

void TextEditor::closeLineWidget()
{
    if (!d->lineWidgetContainer->isVisible())
        return;

    d->lineWidgetContainer->setVisible(false);
    clearAnnotations(d->showAtLine - 1);
    editor.inlineWidgetClosed();
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
            if (hasBreakpoint(line))
                removeBreakpoint(line);
            else
                addBreakpoint(line);
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

    d->delayCursorTimer.start();
    editor.cursorPositionChanged(d->fileName, line, index);
    int pos = positionFromLineIndex(line, index);

    if (!d->contentsChanged && d->lastCursorNeedRecord) {
        emit cursorRecordChanged(d->lastCursorPos);
        d->lastCursorNeedRecord = false;
    } else if (d->contentsChanged || d->postionChangedByGoto) {
        d->lastCursorNeedRecord = true;
        d->postionChangedByGoto = false;
    }
    d->lastCursorPos = pos;
}

void TextEditor::focusOutEvent(QFocusEvent *event)
{
    if (!d->lineWidgetContainer->hasFocus())
        closeLineWidget();

    d->cancelInlineCompletion();
    Q_EMIT focusOut();
    Q_EMIT followTypeEnd();
    QsciScintilla::focusOutEvent(event);
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab && d->inlineCompletionCache.first != -1) {
        d->applyInlineCompletion();
        auto providerList = ResourceManager::instance()->inlineCompletionProviders();
        for (const auto provider : providerList)
            provider->accepted();
        return;
    }

    if (event->key() == Qt::Key_Escape && d->lineWidgetContainer->isVisible())
        return closeLineWidget();

    if (d->completionWidget->processKeyPressEvent(event))
        return;

    if (event->key() == Qt::Key_Escape && d->inlineCompletionCache.first != -1) {
        d->cancelInlineCompletion();
        auto providerList = ResourceManager::instance()->inlineCompletionProviders();
        for (const auto provider : providerList)
            provider->rejected();
        return;
    }

    QsciScintilla::keyPressEvent(event);
}

void TextEditor::mouseMoveEvent(QMouseEvent *event)
{
    bool isCtrlPressed = qApp->queryKeyboardModifiers().testFlag(Qt::ControlModifier);
    if (isCtrlPressed) {
        auto point = event->pos();
        auto pos = positionFromPoint(point.x(), point.y());
        Q_EMIT requestFollowType(pos);
    }

    QsciScintilla::mouseMoveEvent(event);
}

void TextEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton))
        d->leftButtonPressed = true;

    QsciScintilla::mousePressEvent(event);
}

void TextEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton))
        d->leftButtonPressed = false;

    QsciScintilla::mouseReleaseEvent(event);
}

bool TextEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->lineWidgetContainer && event->type() == QEvent::Resize) {
        d->updateLineWidgetPosition();
    } else if (obj == d->mainWindow() && event->type() == QEvent::Move) {
        d->updateLineWidgetPosition();
    } else if (obj == d->completionWidget && event->type() == QEvent::Hide) {
        d->updateInlineCompletion();
    }

    return QsciScintilla::eventFilter(obj, event);
}

bool TextEditor::event(QEvent *event)
{
    if (!d)
        return QsciScintilla::event(event);

    if (event->type() != QEvent::InputMethodQuery)
        d->contentsChanged = false;

    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent) {
            bool isCtrlPressed = keyEvent->modifiers().testFlag(Qt::ControlModifier);
            if (!isCtrlPressed)
                Q_EMIT followTypeEnd();
        }
    }

    return QsciScintilla::event(event);
}

void TextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    if (!contextMenuNeeded(event->pos().x(), event->pos().y()))
        return;

    d->tipsDisplayable = false;
    int xPos = event->pos().x();
    if (xPos <= d->marginsWidth()) {
        d->showMarginMenu();
    } else {
        d->showContextMenu();
    }
    d->tipsDisplayable = true;
}

void TextEditor::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const auto &urlList = mimeData->urls();
        Q_EMIT requestOpenFiles(urlList);
        return;
    }

    QsciScintilla::dropEvent(event);
}

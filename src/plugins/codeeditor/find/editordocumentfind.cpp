// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editordocumentfind.h"
#include "gui/tabwidget.h"
#include "gui/texteditor.h"

#define SCFIND_REGEXP_DOTMATCHESNL 0x10000000
#define SCFIND_REGEXP_EMPTYMATCH_MASK 0xE0000000
#define SCFIND_REGEXP_EMPTYMATCH_NONE 0x00000000
#define SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH 0x20000000
#define SCFIND_REGEXP_EMPTYMATCH_ALL 0x40000000
#define SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART 0x80000000
#define SCFIND_REGEXP_SKIPCRLFASONE 0x08000000

class EditorDocumentFindPrivate
{
public:
    struct FindReplaceInfo
    {
        intptr_t startRange = -1;
        intptr_t endRange = -1;
    };

    explicit EditorDocumentFindPrivate(EditorDocumentFind *qq);

    TabWidget *tabWidget() const;
    QWidget *autoAdjustCurrentEditor();
    void adjustFindStartPosition(TextEditor *editor);
    void dealWithZeroFound(TextEditor *editor);
    bool findStep(const QString &text, bool isForward);
    void doReplaceAll(TextEditor *editor, const QString &findText,
                      const QString &replaceText, bool caseSensitive = false, bool wholeWords = false);
    // int buildSearchFlags(bool re, bool cs, bool wo, bool wrap, bool forward, FindNextType findNextType, bool posix, bool cxx11);

public:
    EditorDocumentFind *q;
    bool isFindFirst { true };
    bool isReverseFind { false };
    QWidget *curEditor { nullptr };
};

EditorDocumentFindPrivate::EditorDocumentFindPrivate(EditorDocumentFind *qq)
    : q(qq)
{
}

TabWidget *EditorDocumentFindPrivate::tabWidget() const
{
    auto tw = qobject_cast<TabWidget *>(q->parent());
    Q_ASSERT(tw);

    return tw;
}

QWidget *EditorDocumentFindPrivate::autoAdjustCurrentEditor()
{
    auto w = tabWidget()->currentEditor();
    if (w != curEditor) {
        curEditor = w;
        isFindFirst = true;
    }

    return w;
}

void EditorDocumentFindPrivate::adjustFindStartPosition(TextEditor *editor)
{
    // int pos = editor->cursorPosition();
    // FindState &state = editor->getLastFindState();
    // if (state.targend != pos)
    //     state.startpos = pos;
}

void EditorDocumentFindPrivate::dealWithZeroFound(TextEditor *editor)
{
    // FindState &state = editor->getLastFindState();
    // if (state.targstart == state.targend)
    //     state.startpos++;
}

bool EditorDocumentFindPrivate::findStep(const QString &text, bool isForward)
{
    if (text.isEmpty())
        return false;

    auto w = autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor)
        return false;

    bool ret = false;
    if (isFindFirst) {
        int line = -1, index = -1;
        // For forward search, 'index' needs to subtract the length of 'keyword',
        // otherwise it cannot jump to the previous one
        if (!isForward) {
            editor->getCursorPosition(&line, &index);
            index -= text.length();
        }

        ret = editor->findFirst(text, false, false, false, true, isForward, line, index);
        isFindFirst = !ret;
        if (ret)
            dealWithZeroFound(editor);
    } else {
        adjustFindStartPosition(editor);
        ret = editor->findNext();
        if (!ret)
            isFindFirst = true;
        else
            dealWithZeroFound(editor);
    }

    return ret;
}

void EditorDocumentFindPrivate::doReplaceAll(TextEditor *editor, const QString &findText,
                                             const QString &replaceText, bool caseSensitive, bool wholeWords)
{
    int srcPosition = editor->cursorPosition();
    int firstDisLineNum = editor->SendScintilla(TextEditor::SCI_GETFIRSTVISIBLELINE);
    editor->beginUndoAction();

    // int flags = buildSearchFlags(false, caseSensitive, wholeWords, false, true, FINDNEXTTYPE_REPLACENEXT, 0, 0);
    // editor->SendScintilla(TextEditor::SCI_SETSEARCHFLAGS, flags);

    FindReplaceInfo findReplaceInfo;
    findReplaceInfo.startRange = 0;
    findReplaceInfo.endRange = editor->SendScintilla(TextEditor::SCI_GETLENGTH);

    QByteArray textFind = findText.toUtf8();
    QByteArray textReplace = replaceText.toUtf8();

    intptr_t targetStart = 0;
    intptr_t targetEnd = 0;
    while (targetStart >= 0) {
        targetStart = editor->searchInTarget(textFind, findReplaceInfo.startRange, findReplaceInfo.endRange);
        // If we've not found anything, just break out of the loop
        if (targetStart == -1 || targetStart == -2)
            break;

        targetEnd = editor->SendScintilla(TextEditor::SCI_GETTARGETEND);
        if (targetEnd > findReplaceInfo.endRange)
            break;

        intptr_t foundTextLen = targetEnd - targetStart;
        intptr_t replacedLength = editor->replaceTarget(textReplace);
        intptr_t replaceDelta = replacedLength - foundTextLen;
        // After the processing of the last string occurrence the search loop should be stopped
        // This helps to avoid the endless replacement during the EOL ("$") searching
        if (targetStart + foundTextLen == findReplaceInfo.endRange)
            break;

        findReplaceInfo.startRange = targetStart + foundTextLen + replaceDelta;   //search from result onwards
        findReplaceInfo.endRange += replaceDelta;   //adjust end of range in case of replace
    }

    editor->endUndoAction();
    editor->gotoPosition(srcPosition);
    editor->setFirstVisibleLine(firstDisLineNum);
}

// int EditorDocumentFindPrivate::buildSearchFlags(bool re, bool cs, bool wo, bool wrap, bool forward, FindNextType findNextType, bool posix, bool cxx11)
// {
//     int flags = 0;

//     flags = (cs ? TextEditor::SCFIND_MATCHCASE : 0)
//             | (wo ? TextEditor::SCFIND_WHOLEWORD : 0)
//             | (re ? TextEditor::SCFIND_REGEXP : 0)
//             | (posix ? TextEditor::SCFIND_POSIX : 0)
//             | (cxx11 ? TextEditor::SCFIND_CXX11REGEX : 0);

//     switch (findNextType) {
//     case FINDNEXTTYPE_FINDNEXT:
//         flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_SKIPCRLFASONE;
//         break;

//     case FINDNEXTTYPE_REPLACENEXT:
//         flags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_SKIPCRLFASONE;
//         break;

//     case FINDNEXTTYPE_FINDNEXTFORREPLACE:
//         flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_SKIPCRLFASONE;
//         break;
//     }
//     return flags;
// }

EditorDocumentFind::EditorDocumentFind(QObject *parent)
    : AbstractDocumentFind(parent),
      d(new EditorDocumentFindPrivate(this))
{
}

EditorDocumentFind::~EditorDocumentFind()
{
    delete d;
}

QString EditorDocumentFind::findString() const
{
    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor)
        return {};

    QString findText = editor->selectedText();
    // if (findText.isEmpty())
    //     findText = editor->wordAtPosition(editor->cursorPosition());

    return findText;
}

void EditorDocumentFind::findNext(const QString &txt)
{
    if (d->isReverseFind) {
        d->isReverseFind = false;
        d->isFindFirst = true;
    }

    d->findStep(txt, true);
}

void EditorDocumentFind::findPrevious(const QString &txt)
{
    if (!d->isReverseFind) {
        d->isReverseFind = true;
        d->isFindFirst = true;
    }

    d->findStep(txt, false);
}

void EditorDocumentFind::replace(const QString &before, const QString &after)
{
    if (before.isEmpty())
        return;

    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor || editor->isReadOnly())
        return;

    const auto &selText = editor->selectedText();
    if (!selText.isEmpty() && selText == before)
        editor->replaceSelectedText(after);
}

void EditorDocumentFind::replaceFind(const QString &before, const QString &after)
{
    d->isFindFirst = true;
    replace(before, after);
    findNext(before);
}

void EditorDocumentFind::replaceAll(const QString &before, const QString &after)
{
    if (before.isEmpty())
        return;

    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor || editor->isReadOnly())
        return;

    d->doReplaceAll(editor, before, after);
    d->isFindFirst = true;
}

void EditorDocumentFind::findStringChanged()
{
    d->isFindFirst = true;
}

void EditorDocumentFind::replaceAll(TextEditor *editor, const QString &before,
                                    const QString &after, bool caseSensitive, bool wholeWords)
{
    if (before.isEmpty())
        return;

    if (!editor || editor->isReadOnly())
        return;

    d->doReplaceAll(editor, before, after, caseSensitive, wholeWords);
    d->isFindFirst = true;
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findtoolbar.h"
#include "gui/tabwidget.h"
#include "gui/texteditor.h"

#include <DLineEdit>
#include <DToolButton>

#include <QLabel>
#include <QPainter>
#include <QStyleOptionToolBar>
#include <QStyleOptionTitleBar>
#include <QCoreApplication>

#define SCFIND_REGEXP_DOTMATCHESNL 0x10000000
#define SCFIND_REGEXP_EMPTYMATCH_MASK 0xE0000000
#define SCFIND_REGEXP_EMPTYMATCH_NONE 0x00000000
#define SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH 0x20000000
#define SCFIND_REGEXP_EMPTYMATCH_ALL 0x40000000
#define SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART 0x80000000
#define SCFIND_REGEXP_SKIPCRLFASONE 0x08000000

DWIDGET_USE_NAMESPACE

class FindToolBarPrivate
{
public:
    struct Tr
    {
        Q_DECLARE_TR_FUNCTIONS(QtC::Core)
    };

    struct FindReplaceInfo
    {
        intptr_t startRange = -1;
        intptr_t endRange = -1;
    };

    explicit FindToolBarPrivate(FindToolBar *qq);

    void initUI();
    void initConnection();

    TabWidget *tabWidget() const;
    QWidget *autoAdjustCurrentEditor();
    void adjustFindStartPosition(TextEditor *editor);
    void dealWithZeroFound(TextEditor *editor);
    void doReplaceAll(TextEditor *editor, const QString &findText, const QString &replaceText);
    int buildSearchFlags(bool re, bool cs, bool wo, bool wrap, bool forward, FindNextType findNextType, bool posix, bool cxx11);

public:
    FindToolBar *q;

    DLineEdit *findEdit { nullptr };
    DToolButton *findPreviousButton { nullptr };
    DToolButton *findNextButton { nullptr };
    DToolButton *closeButton { nullptr };

    DLineEdit *replaceEdit { nullptr };
    DToolButton *replaceButton { nullptr };
    DToolButton *replaceNextButton { nullptr };
    DToolButton *replaceAllButton { nullptr };

    bool isFindFirst { true };
    bool isReverseFind { false };
    QWidget *curEditor { nullptr };
};

FindToolBarPrivate::FindToolBarPrivate(FindToolBar *qq)
    : q(qq)
{
}

void FindToolBarPrivate::initUI()
{
    q->setAutoFillBackground(true);
    q->setBackgroundRole(QPalette::Base);

    auto *findLabel = new QLabel(Tr::tr("Find:"), q);
    findEdit = new DLineEdit(q);
    findEdit->setMinimumWidth(100);

    findPreviousButton = new DToolButton(q);
    findPreviousButton->setText(Tr::tr("Find Previous"));

    findNextButton = new DToolButton(q);
    findNextButton->setText(Tr::tr("Find Next"));

    closeButton = new DToolButton(q);
    closeButton->setIcon(QIcon::fromTheme("edit-closeBtn"));

    auto findButtonsWidget = new QWidget(q);
    auto findButtonLayout = new QHBoxLayout(findButtonsWidget);
    findButtonLayout->setSpacing(3);
    findButtonLayout->setContentsMargins(0, 0, 0, 0);
    findButtonLayout->addWidget(findPreviousButton);
    findButtonLayout->addWidget(findNextButton);
    findButtonLayout->addSpacerItem(new QSpacerItem(40, 0, QSizePolicy::Expanding));
    findButtonLayout->addWidget(closeButton);

    auto replaceLabel = new QLabel(Tr::tr("Replace with:"), q);
    replaceEdit = new DLineEdit(q);
    replaceEdit->setMinimumWidth(100);

    replaceButton = new DToolButton(q);
    replaceButton->setText(Tr::tr("Replace"));
    replaceButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    replaceButton->setArrowType(Qt::LeftArrow);

    replaceNextButton = new DToolButton(q);
    replaceNextButton->setText(Tr::tr("Replace && Find"));
    replaceNextButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    replaceNextButton->setArrowType(Qt::RightArrow);

    replaceAllButton = new DToolButton(q);
    replaceAllButton->setText(Tr::tr("Replace All"));
    replaceAllButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    auto replaceButtonsWidget = new QWidget;
    replaceButtonsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto replaceButtonsLayout = new QHBoxLayout(replaceButtonsWidget);
    replaceButtonsLayout->setSpacing(3);
    replaceButtonsLayout->setContentsMargins(0, 0, 0, 0);
    replaceButtonsLayout->addWidget(replaceButton);
    replaceButtonsLayout->addWidget(replaceNextButton);
    replaceButtonsLayout->addWidget(replaceAllButton);

    auto mainLayout = new QGridLayout(q);
    mainLayout->setHorizontalSpacing(3);
    mainLayout->setVerticalSpacing(1);
    mainLayout->setContentsMargins(5, 2, 0, 1);
    mainLayout->setColumnStretch(1, 10);

    mainLayout->addWidget(findLabel, 0, 0);
    mainLayout->addWidget(findEdit, 0, 1);
    mainLayout->addWidget(findButtonsWidget, 0, 2);
    mainLayout->addWidget(replaceLabel, 1, 0);
    mainLayout->addWidget(replaceEdit, 1, 1);
    mainLayout->addWidget(replaceButtonsWidget, 1, 2);
}

void FindToolBarPrivate::initConnection()
{
    q->connect(closeButton, &DToolButton::clicked, q, &FindToolBar::hide);
    q->connect(findPreviousButton, &DToolButton::clicked, q, &FindToolBar::invokeFindPrevious);
    q->connect(findNextButton, &DToolButton::clicked, q, &FindToolBar::invokeFindNext);
    q->connect(replaceButton, &DToolButton::clicked, q, &FindToolBar::invokeReplace);
    q->connect(replaceNextButton, &DToolButton::clicked, q, &FindToolBar::invokeReplaceNext);
    q->connect(replaceAllButton, &DToolButton::clicked, q, &FindToolBar::invokeReplaceAll);
    q->connect(findEdit, &DLineEdit::textChanged, q, &FindToolBar::onFindTextChanged);
}

TabWidget *FindToolBarPrivate::tabWidget() const
{
    auto tw = qobject_cast<TabWidget *>(q->parent());
    Q_ASSERT(tw);

    return tw;
}

QWidget *FindToolBarPrivate::autoAdjustCurrentEditor()
{
    auto w = tabWidget()->currentWidget();
    if (w != curEditor) {
        curEditor = w;
        isFindFirst = true;
    }

    return w;
}

void FindToolBarPrivate::adjustFindStartPosition(TextEditor *editor)
{
    int pos = editor->cursorPosition();
    FindState &state = editor->getLastFindState();
    if (state.targend != pos)
        state.startpos = pos;
}

void FindToolBarPrivate::dealWithZeroFound(TextEditor *editor)
{
    FindState &state = editor->getLastFindState();
    if (state.targstart == state.targend)
        state.startpos++;
}

void FindToolBarPrivate::doReplaceAll(TextEditor *editor, const QString &findText, const QString &replaceText)
{
    int srcPosition = editor->cursorPosition();
    int firstDisLineNum = editor->SendScintilla(TextEditor::SCI_GETFIRSTVISIBLELINE);
    editor->beginUndoAction();

    int flags = buildSearchFlags(false, false, false, false, true, FINDNEXTTYPE_REPLACENEXT, 0, 0);
    editor->SendScintilla(TextEditor::SCI_SETSEARCHFLAGS, flags);

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

int FindToolBarPrivate::buildSearchFlags(bool re, bool cs, bool wo, bool wrap, bool forward, FindNextType findNextType, bool posix, bool cxx11)
{
    int flags = 0;

    flags = (cs ? TextEditor::SCFIND_MATCHCASE : 0)
            | (wo ? TextEditor::SCFIND_WHOLEWORD : 0)
            | (re ? TextEditor::SCFIND_REGEXP : 0)
            | (posix ? TextEditor::SCFIND_POSIX : 0)
            | (cxx11 ? TextEditor::SCFIND_CXX11REGEX : 0);

    switch (findNextType) {
    case FINDNEXTTYPE_FINDNEXT:
        flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_SKIPCRLFASONE;
        break;

    case FINDNEXTTYPE_REPLACENEXT:
        flags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_SKIPCRLFASONE;
        break;

    case FINDNEXTTYPE_FINDNEXTFORREPLACE:
        flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_SKIPCRLFASONE;
        break;
    }
    return flags;
}

FindToolBar::FindToolBar(QWidget *parent)
    : QFrame(parent),
      d(new FindToolBarPrivate(this))
{
    d->initUI();
    d->initConnection();
}

FindToolBar::~FindToolBar()
{
    delete d;
}

void FindToolBar::setFindText(const QString &text)
{
    d->findEdit->setText(text);
    d->findEdit->lineEdit()->selectAll();
    d->findEdit->setFocus();
}

void FindToolBar::invokeFindNext()
{
    if (d->isReverseFind) {
        d->isReverseFind = false;
        d->isFindFirst = true;
    }

    invokeFindStep(true);
}

void FindToolBar::invokeFindPrevious()
{
    if (!d->isReverseFind) {
        d->isReverseFind = true;
        d->isFindFirst = true;
    }

    invokeFindStep(false);
}

bool FindToolBar::invokeFindStep(bool isForward)
{
    auto findText = d->findEdit->text();
    if (findText.isEmpty())
        return false;

    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor)
        return false;

    bool ret = false;
    if (d->isFindFirst) {
        int line = -1, index = -1;
        // For forward search, 'index' needs to subtract the length of 'keyword',
        // otherwise it cannot jump to the previous one
        if (!isForward) {
            editor->getCursorPosition(&line, &index);
            index -= findText.length();
        }

        ret = editor->findFirst(findText, false, false, false, true, isForward, FINDNEXTTYPE_FINDNEXT, line, index);
        d->isFindFirst = !ret;
        if (ret)
            d->dealWithZeroFound(editor);
    } else {
        d->adjustFindStartPosition(editor);
        ret = editor->findNext();
        if (!ret)
            d->isFindFirst = true;
        else
            d->dealWithZeroFound(editor);
    }

    return ret;
}

void FindToolBar::invokeReplace()
{
    auto findText = d->findEdit->text();
    if (findText.isEmpty())
        return;

    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor || editor->isReadOnly())
        return;

    const auto &selText = editor->selectedText();
    if (!selText.isEmpty() && selText == findText)
        editor->replaceSelectedText(d->replaceEdit->text());
}

void FindToolBar::invokeReplaceNext()
{
    d->isFindFirst = true;
    invokeReplace();
    invokeFindNext();
}

void FindToolBar::invokeReplaceAll()
{
    auto findText = d->findEdit->text();
    if (findText.isEmpty())
        return;

    auto w = d->autoAdjustCurrentEditor();
    auto editor = qobject_cast<TextEditor *>(w);
    if (!editor || editor->isReadOnly())
        return;

    auto replaceText = d->replaceEdit->text();
    d->doReplaceAll(editor, findText, replaceText);
    d->isFindFirst = true;
}

void FindToolBar::onFindTextChanged()
{
    d->isFindFirst = true;
}

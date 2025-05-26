// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "outputpane.h"

#include <DPlainTextEdit>
#include <DScrollBar>
#include <DMenu>
#include <DGuiApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>
#include <QTimer>
#include <QTextBlock>
#include <QRegularExpression>

/**
 * @brief Output text color.
 */
const QColor kErrorMessageTextColor(255, 108, 108);
const QColor kMessageOutput(0, 135, 135);
constexpr int kDefaultMaxCharCount = 10000000;

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class OutputPanePrivate
{
public:
    struct Output
    {
        QString text;
        OutputPane::AppendMode mode;
        OutputPane::OutputFormat format;
    };

    explicit OutputPanePrivate(){};

public:
    bool enforceNewline = false;
    bool scrollToBottom = true;
    int maxCharCount = kDefaultMaxCharCount;
    QTextCursor cursor;
    DPlainTextEdit *outputEdit = nullptr;
    DMenu *menu = nullptr;

    QList<Output> outputList;
    QTimer outputTimer;

    QString filterText;
    int lastBlockNumber = -1;
};

OutputPane::OutputPane(QWidget *parent)
    : QWidget(parent),
      d(new OutputPanePrivate())
{
    initUI();
    initTimer();
}

OutputPane::~OutputPane()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void OutputPane::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    d->outputEdit = new DPlainTextEdit(this);
    d->outputEdit->setLineWidth(0);
    d->outputEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

    d->cursor = QTextCursor(d->outputEdit->document());

    d->menu = new DMenu(this);
    d->menu->addActions(actionFactory());

    mainLayout->addWidget(d->outputEdit);
}

void OutputPane::initTimer()
{
    d->outputTimer.setSingleShot(true);
    d->outputTimer.setInterval(100);
    connect(&d->outputTimer, &QTimer::timeout, this, &OutputPane::handleNextOutput);
}

void OutputPane::clearContents()
{
    d->outputEdit->clear();
}

QString OutputPane::normalizeNewlines(const QString &text)
{
    QString res = text;
    res.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    return res;
}

void OutputPane::handleNextOutput()
{
    auto &output = d->outputList.first();
    QTextCharFormat textFormat;
    switch (output.format) {
    case OutputFormat::StdOut:
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::StdErr:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::NormalMessage:
        textFormat.setForeground(kMessageOutput);
        break;
    case OutputFormat::ErrorMessage:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Bold);
        break;
    default:
        textFormat.setFontWeight(QFont::Normal);
    }

    if (output.text.size() <= d->maxCharCount) {
        appendCustomText(output.text, output.mode, textFormat);
        d->outputList.removeFirst();
    } else {
        appendCustomText(output.text.left(d->maxCharCount), output.mode, textFormat);
        output.text.remove(0, d->maxCharCount);
    }

    if (!d->outputList.isEmpty())
        d->outputTimer.start();
}

bool OutputPane::isScrollbarAtBottom() const
{
    return d->outputEdit->verticalScrollBar()->value() == d->outputEdit->verticalScrollBar()->maximum();
}

QString OutputPane::doNewlineEnforcement(const QString &out)
{
    d->scrollToBottom = true;
    QString s = out;
    if (d->enforceNewline) {
        s.prepend(QLatin1Char('\n'));
        d->enforceNewline = false;
    }

    if (s.endsWith(QLatin1Char('\n'))) {
        d->enforceNewline = true;   // make appendOutputInline put in a newline next time
        s.chop(1);
    }

    return s;
}

void OutputPane::scrollToBottom()
{
    d->outputEdit->verticalScrollBar()->setValue(d->outputEdit->verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    d->outputEdit->verticalScrollBar()->setValue(d->outputEdit->verticalScrollBar()->maximum());
}

void OutputPane::appendCustomText(const QString &textIn, AppendMode mode, const QTextCharFormat &format)
{
    if (d->maxCharCount > 0 && d->outputEdit->document()->characterCount() >= d->maxCharCount) {
        qDebug() << "Maximum limit exceeded : " << d->maxCharCount;
        return;
    }

    if (!d->cursor.atEnd())
        d->cursor.movePosition(QTextCursor::End);

    if (mode == OverWrite) {
        d->cursor.select(QTextCursor::LineUnderCursor);
        d->cursor.removeSelectedText();
    }

    auto text = mode == OverWrite ? textIn.trimmed() : normalizeNewlines(doNewlineEnforcement(textIn));
    d->cursor.insertText(text, format);

    if (d->maxCharCount > 0 && d->outputEdit->document()->characterCount() >= d->maxCharCount) {
        QTextCharFormat tmp;
        tmp.setFontWeight(QFont::Bold);
        d->cursor.insertText(doNewlineEnforcement(tr("Additional output omitted") + QLatin1Char('\n')), tmp);
    }

    if (!d->filterText.isEmpty())
        filterContent(false, false);

    scrollToBottom();
}

void OutputPane::appendText(const QString &text, OutputFormat format, AppendMode mode)
{
    if (d->outputList.isEmpty()
        || d->outputList.last().mode != mode
        || d->outputList.last().format != format) {
        d->outputList.append({ text, mode, format });
    } else {
        auto &output = d->outputList.last();
        output.text.append(text);
    }

    if (!d->outputTimer.isActive())
        d->outputTimer.start();
}

QTextDocument *OutputPane::document() const
{
    return d->outputEdit->document();
}

QPlainTextEdit *OutputPane::edit() const
{
    return d->outputEdit;
}

OutputPane *OutputPane::instance()
{
    static OutputPane *ins = new OutputPane();
    return ins;
}

void OutputPane::contextMenuEvent(QContextMenuEvent *event)
{
    if (!d->menu)
        return;

    d->menu->move(event->globalX(), event->globalY());
    d->menu->show();
}

QList<QAction *> OutputPane::actionFactory()
{
    QList<QAction *> list;

    {
        auto action = new QAction(this);
        action->setText(tr("Copy"));
        action->setEnabled(false);
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->copy();
        });
        connect(d->outputEdit, &QPlainTextEdit::copyAvailable, action, &QAction::setEnabled);
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Clear"));
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->clear();
        });
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Select All"));
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->selectAll();
        });
        list.append(action);
    }

    return list;
}

void OutputPane::updateFilter(const QString &filterText, bool caseSensitive, bool isRegexp)
{
    d->outputEdit->setReadOnly(!filterText.isEmpty());

    d->filterText = filterText;
    d->lastBlockNumber = -1;
    filterContent(caseSensitive, isRegexp);
}

void OutputPane::filterContent(bool caseSensitive, bool isRegexp)
{
    auto document = d->outputEdit->document();
    auto lastBlock = document->findBlockByLineNumber(d->lastBlockNumber);
    if (!lastBlock.isValid())
        lastBlock = document->begin();

    if (isRegexp) {
        QRegularExpression regExp(d->filterText);
        if (!caseSensitive)
            regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

        for (; lastBlock != document->end(); lastBlock = lastBlock.next())
            lastBlock.setVisible(d->filterText.isEmpty() || regExp.match(lastBlock.text()).hasMatch());
    } else {
        if (caseSensitive) {
            for (; lastBlock != document->end(); lastBlock = lastBlock.next())
                lastBlock.setVisible(d->filterText.isEmpty() || lastBlock.text().contains(d->filterText));
        } else {
            for (; lastBlock != document->end(); lastBlock = lastBlock.next())
                lastBlock.setVisible(d->filterText.isEmpty() || lastBlock.text().toLower().contains(d->filterText.toLower()));
        }
    }

    d->lastBlockNumber = d->outputEdit->document()->lastBlock().blockNumber();
    d->outputEdit->setDocument(this->document());

    scrollToBottom();
}

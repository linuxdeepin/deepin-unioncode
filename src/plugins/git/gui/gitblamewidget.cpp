// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitblamewidget.h"
#include "giteditor.h"
#include "constants.h"
#include "highlighter/gitblamehighlighter.h"

#include "common/util/qtcassert.h"

#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QRegularExpression>

DGUI_USE_NAMESPACE

class GitBlameWidgetPrivate : public QObject
{
public:
    explicit GitBlameWidgetPrivate(GitBlameWidget *qq);

    void initUI();
    void initConnection();

    void updateHighlighter();
    QSet<QString> annotationChanges() const;

    void regexpFromString(const QString &pattern,
                          QRegularExpression *regexp,
                          QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption);

public:
    GitBlameWidget *q;

    QRegularExpression annotationEntryPattern;
    GitEditor *gitEditor { nullptr };
    GitBlameHighlighter *highlighter { nullptr };
};

GitBlameWidgetPrivate::GitBlameWidgetPrivate(GitBlameWidget *qq)
    : q(qq)
{
    regexpFromString(AnnotationEntryPattern, &annotationEntryPattern, QRegularExpression::MultilineOption);
}

void GitBlameWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    gitEditor = new GitEditor(q);
    gitEditor->setReadOnly(true);
    gitEditor->setTextInteractionFlags(gitEditor->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    mainLayout->addWidget(gitEditor);
}

void GitBlameWidgetPrivate::initConnection()
{
    connect(gitEditor, &QPlainTextEdit::textChanged, this, &GitBlameWidgetPrivate::updateHighlighter);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GitBlameWidgetPrivate::updateHighlighter);
}

void GitBlameWidgetPrivate::updateHighlighter()
{
    const QSet<QString> changes = annotationChanges();
    if (changes.isEmpty())
        return;

    if (highlighter)
        delete highlighter;

    highlighter = new GitBlameHighlighter(changes);
    highlighter->setParent(this);
    highlighter->setDocument(gitEditor->document());
}

QSet<QString> GitBlameWidgetPrivate::annotationChanges() const
{
    QSet<QString> changes;
    const QString text = gitEditor->toPlainText();
    if (text.isEmpty())
        return changes;

    QRegularExpressionMatchIterator i = annotationEntryPattern.globalMatch(text);
    while (i.hasNext()) {
        const QRegularExpressionMatch match = i.next();
        changes.insert(match.captured(1));
    }
    return changes;
}

void GitBlameWidgetPrivate::regexpFromString(const QString &pattern,
                                             QRegularExpression *regexp,
                                             QRegularExpression::PatternOptions options)
{
    const QRegularExpression re(pattern, options);
    QTC_ASSERT(re.isValid() && re.captureCount() >= 1, return );
    *regexp = re;
}

GitBlameWidget::GitBlameWidget(QWidget *parent)
    : GitBaseWidget(parent),
      d(new GitBlameWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

GitBlameWidget::~GitBlameWidget()
{
    delete d;
}

void GitBlameWidget::setGitInfo(const QStringList &infos)
{
    if (infos.isEmpty())
        return d->gitEditor->clear();

    const auto info = infos.first();
    const QChar space(' ');
    const int parenPos = info.indexOf(')');
    if (parenPos == -1)
        return d->gitEditor->setPlainText(info);

    int i = parenPos;
    while (i >= 0 && info.at(i) != space)
        --i;
    while (i >= 0 && info.at(i) == space)
        --i;
    int stripPos = i + 1;

    // Copy over the parts that have not changed into a new byte array
    QString result;
    int prevPos = 0;
    int pos = info.indexOf('\n', 0) + 1;
    forever {
        QTC_CHECK(prevPos < pos);
        int afterParen = prevPos + parenPos;
        result.append(info.midRef(prevPos, stripPos));
        result.append(info.midRef(afterParen, pos - afterParen));
        prevPos = pos;
        QTC_CHECK(prevPos != 0);
        if (pos == info.size())
            break;

        pos = info.indexOf('\n', pos) + 1;
        if (pos == 0)   // indexOf returned -1
            pos = info.size();
    }

    d->gitEditor->setPlainText(result);
}

void GitBlameWidget::setReadyMessage(const QString &msg)
{
    d->gitEditor->setPlainText(msg);
}

void GitBlameWidget::setSourceFile(const QString &sf)
{
    d->gitEditor->setSourceFile(sf);
}

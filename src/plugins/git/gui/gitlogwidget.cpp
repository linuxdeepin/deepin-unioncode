// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitlogwidget.h"
#include "giteditor.h"
#include "utils/codeformathandler.h"

#include <QVBoxLayout>

class GitLogWidgetPrivate : public QObject
{
public:
    explicit GitLogWidgetPrivate(GitLogWidget *qq);

    void initUI();

public:
    GitLogWidget *q;

    GitEditor *gitEditor { nullptr };
};

GitLogWidgetPrivate::GitLogWidgetPrivate(GitLogWidget *qq)
    : q(qq)
{
}

void GitLogWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    gitEditor = new GitEditor(q);
    gitEditor->setReadOnly(true);
    gitEditor->setTextInteractionFlags(gitEditor->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    mainLayout->addWidget(gitEditor);
}

GitLogWidget::GitLogWidget(QWidget *parent)
    : GitBaseWidget(parent),
      d(new GitLogWidgetPrivate(this))
{
    d->initUI();
}

GitLogWidget::~GitLogWidget()
{
    delete d;
}

void GitLogWidget::setGitInfo(const QString &info)
{
    CodeFormatHandler handler;
    const QList<FormattedText> formattedTextList = handler.parseText(FormattedText(info));

    d->gitEditor->clear();
    QTextCursor cursor = d->gitEditor->textCursor();
    cursor.beginEditBlock();
    for (const auto &formattedChunk : formattedTextList)
        cursor.insertText(formattedChunk.text, formattedChunk.format);
    cursor.endEditBlock();
    d->gitEditor->document()->setModified(false);

    cursor.setPosition(0);
    d->gitEditor->setTextCursor(cursor);
}

void GitLogWidget::setReadyMessage(const QString &msg)
{
    d->gitEditor->setPlainText(msg);
}

void GitLogWidget::setSourceFile(const QString &sf)
{
    d->gitEditor->setSourceFile(sf);
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitshowwidget.h"
#include "gitdiffwidget.h"

#include <QPlainTextEdit>
#include <QSplitter>
#include <QVBoxLayout>

class GitShowWidgetPrivate
{
public:
    explicit GitShowWidgetPrivate(GitShowWidget *qq);

    void initUI();

public:
    GitShowWidget *q;

    QPlainTextEdit *commitInfoEdit { nullptr };
    GitDiffWidget *gitDiffWidget { nullptr };
};

GitShowWidgetPrivate::GitShowWidgetPrivate(GitShowWidget *qq)
    : q(qq)
{
}

void GitShowWidgetPrivate::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Vertical, q);
    commitInfoEdit = new QPlainTextEdit(q);
    commitInfoEdit->setReadOnly(true);
    commitInfoEdit->setTextInteractionFlags(commitInfoEdit->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    gitDiffWidget = new GitDiffWidget(q);

    splitter->addWidget(commitInfoEdit);
    splitter->addWidget(gitDiffWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    layout->addWidget(splitter);
}

GitShowWidget::GitShowWidget(QWidget *parent)
    : GitBaseWidget(parent),
      d(new GitShowWidgetPrivate(this))
{
    d->initUI();
}

GitShowWidget::~GitShowWidget()
{
    delete d;
}

void GitShowWidget::setGitInfo(const QStringList &infos)
{
    if (infos.size() < 2) {
        d->commitInfoEdit->setPlainText("");
        d->gitDiffWidget->setGitInfo({ "" });
        return;
    }

    auto tmpInfos = infos;
    d->commitInfoEdit->setPlainText(tmpInfos.takeFirst());
    d->gitDiffWidget->setGitInfo({ tmpInfos.join("") });
}

void GitShowWidget::setReadyMessage(const QString &msg)
{
    d->commitInfoEdit->setPlainText(msg);
    d->gitDiffWidget->setReadyMessage(msg);
}

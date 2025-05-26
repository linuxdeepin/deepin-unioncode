// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gittabwidget.h"
#include "gitlogwidget.h"
#include "gitblamewidget.h"
#include "gitdiffwidget.h"
#include "gitshowwidget.h"

#include "services/editor/editorservice.h"

#include <DTabBar>
#include <DToolButton>

#include <QFileInfo>
#include <QStackedWidget>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class GitTabWidgetPrivate : public QObject
{
public:
    explicit GitTabWidgetPrivate(GitTabWidget *qq);

    void initUI();
    void initConnection();

    int indexOf(const QString &tip);
    GitBaseWidget *createWidget(GitType type);

public Q_SLOTS:
    void tabSwitched(int index);
    void tabClosed(int index);

public:
    GitTabWidget *q;

    DTabBar *tabBar { nullptr };
    DToolButton *closeBtn { nullptr };
    QStackedWidget *stackedWidget { nullptr };
};

GitTabWidgetPrivate::GitTabWidgetPrivate(GitTabWidget *qq)
    : q(qq)
{
}

void GitTabWidgetPrivate::initUI()
{
    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);

    tabBar = new DTabBar(q);
    tabBar->setVisibleAddButton(false);
    tabBar->setTabsClosable(true);
    tabBar->setEnabledEmbedStyle(true);

    closeBtn = new DToolButton(q);
    closeBtn->setIcon(QIcon::fromTheme("edit-closeBtn"));

    stackedWidget = new QStackedWidget(q);
    stackedWidget->setContentsMargins(0, 0, 0, 0);

    tabLayout->addWidget(tabBar, 1);
    tabLayout->addWidget(closeBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addLayout(tabLayout);
    mainLayout->addWidget(stackedWidget);
}

void GitTabWidgetPrivate::initConnection()
{
    connect(tabBar, &DTabBar::currentChanged, this, &GitTabWidgetPrivate::tabSwitched);
    connect(tabBar, &DTabBar::tabCloseRequested, this, &GitTabWidgetPrivate::tabClosed);
    connect(closeBtn, &DToolButton::clicked, this, [] {
        auto editSrv = dpfGetService(EditorService);
        editSrv->switchDefaultWidget();
    });
}

int GitTabWidgetPrivate::indexOf(const QString &tip)
{
    int index = -1;
    for (int i = 0; i < tabBar->count(); ++i) {
        if (tabBar->tabToolTip(i) != tip)
            continue;

        index = i;
        break;
    }

    return index;
}

GitBaseWidget *GitTabWidgetPrivate::createWidget(GitType type)
{
    GitBaseWidget *widget { nullptr };
    switch (type) {
    case GitLog:
        widget = new GitLogWidget(q);
        break;
    case GitBlame:
        widget = new GitBlameWidget(q);
        break;
    case GitDiff:
        widget = new GitDiffWidget(q);
        break;
    case GitShow:
        widget = new GitShowWidget(q);
        break;
    }

    return widget;
}

void GitTabWidgetPrivate::tabSwitched(int index)
{
    stackedWidget->setCurrentIndex(index);
}

void GitTabWidgetPrivate::tabClosed(int index)
{
    tabBar->removeTab(index);
    auto w = stackedWidget->widget(index);
    stackedWidget->removeWidget(w);

    if (stackedWidget->count() == 0) {
        auto editSrv = dpfGetService(EditorService);
        editSrv->switchDefaultWidget();
    }
}

GitTabWidget::GitTabWidget(QWidget *parent)
    : AbstractEditWidget(parent),
      d(new GitTabWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

GitTabWidget::~GitTabWidget()
{
    delete d;
}

int GitTabWidget::addWidget(GitType type, const QString &name)
{
    QFileInfo info(name);
    QString indexStr, title;
    switch (type) {
    case GitLog:
        indexStr = tr("Git Log \"%1\"").arg(name);
        title = tr("Git Log \"%1\"").arg(info.fileName());
        break;
    case GitBlame:
        indexStr = tr("Git Blame \"%1\"").arg(name);
        title = tr("Git Blame \"%1\"").arg(info.fileName());
        break;
    case GitDiff:
        indexStr = tr("Git Diff \"%1\"").arg(name);
        title = tr("Git Diff \"%1\"").arg(info.fileName());
        break;
    case GitShow:
        indexStr = tr("Git Show \"%1\"").arg(name);
        title = indexStr;
        break;
    }

    int index = d->indexOf(indexStr);
    if (index == -1) {
        auto widget = d->createWidget(type);
        if (!widget)
            return -1;

        widget->setSourceFile(name);
        widget->setReadyMessage(tr("Working..."));
        d->stackedWidget->addWidget(widget);
        index = d->tabBar->addTab(title);
        d->tabBar->setTabToolTip(index, indexStr);
    } else {
        auto widget = qobject_cast<GitBaseWidget *>(d->stackedWidget->widget(index));
        if (widget)
            widget->setReadyMessage(tr("Working..."));
    }

    d->tabBar->setCurrentIndex(index);
    return index;
}

void GitTabWidget::setInfo(int index, const QStringList &infos)
{
    if (auto widget = qobject_cast<GitBaseWidget *>(d->stackedWidget->widget(index)))
        widget->setGitInfo(infos);
}

void GitTabWidget::setErrorMessage(int index, const QString &msg)
{
    if (auto widget = qobject_cast<GitBaseWidget *>(d->stackedWidget->widget(index)))
        widget->setReadyMessage(msg);
}

void GitTabWidget::closeWidget()
{
    auto index = d->tabBar->currentIndex();
    Q_EMIT d->tabBar->tabCloseRequested(index);
}

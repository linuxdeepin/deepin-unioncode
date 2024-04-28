// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gittabwidget.h"
#include "gitlogwidget.h"

#include <DTabBar>
#include <DToolButton>

#include <QFileInfo>
#include <QStackedWidget>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class GitTabWidgetPrivate : public QObject
{
public:
    explicit GitTabWidgetPrivate(GitTabWidget *qq);

    void initUI();
    void initConnection();

    int indexOf(const QString &tip);
    int addLogWidget(const QString &path);

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
    connect(closeBtn, &DToolButton::clicked, q, &GitTabWidget::closeRequested);
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

int GitTabWidgetPrivate::addLogWidget(const QString &path)
{
    QFileInfo info(path);
    QString tip(tr("Git Log \"%1\"").arg(path));
    int index = indexOf(tip);
    if (index == -1) {
        QString title = tr("Git Log \"%1\"").arg(info.fileName());
        GitLogWidget *logWidget = new GitLogWidget(q);
        logWidget->textDocument()->setPlainText(tr("Working..."));

        stackedWidget->addWidget(logWidget);
        index = tabBar->addTab(title);
        tabBar->setTabToolTip(index, tip);
    } else {
        auto logWidget = qobject_cast<GitLogWidget *>(stackedWidget->widget(index));
        if (logWidget)
            logWidget->textDocument()->setPlainText(tr("Working..."));
    }

    tabBar->setCurrentIndex(index);
    return index;
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

    if (stackedWidget->count() == 0)
        Q_EMIT q->closeRequested();
}

GitTabWidget::GitTabWidget(QWidget *parent)
    : QWidget(parent),
      d(new GitTabWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

GitTabWidget::~GitTabWidget()
{
    delete d;
}

int GitTabWidget::addWidget(Type type, const QString &path)
{
    switch (type) {
    case GitLog:
        return d->addLogWidget(path);
    case GitBlame:
    case GitDiff:
    default:
        break;
    }

    return -1;
}

void GitTabWidget::setInfo(int index, const QString &info)
{
    if (auto widget = qobject_cast<GitBaseWidget *>(d->stackedWidget->widget(index)))
        widget->setGitInfo(info);
}

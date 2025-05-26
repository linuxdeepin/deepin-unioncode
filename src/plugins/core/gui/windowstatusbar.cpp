// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowstatusbar.h"
#include "progressbar.h"
#include "notify/notificationmanager.h"

#include "common/common.h"

#include <DToolButton>

DWIDGET_USE_NAMESPACE

class WindowStatusBarPrivate
{
    friend class WindowStatusBar;
    ProgressBar *progressBar = nullptr;
    ElidedLabel *messageLabel = nullptr;
    DToolButton *notifyBtn = nullptr;
};

WindowStatusBar::WindowStatusBar(QWidget *parent)
    : DStatusBar(parent), d(new WindowStatusBarPrivate())
{
    d->progressBar = new ProgressBar;
    d->progressBar->setFixedSize(80, 10);
    d->messageLabel = new ElidedLabel;
    d->messageLabel->setMaximumWidth(200);

    d->notifyBtn = new DToolButton(this);
    d->notifyBtn->setIconSize({ 16, 16 });
    d->notifyBtn->setIcon(QIcon::fromTheme("notification"));
    connect(d->notifyBtn, &DToolButton::clicked, this, &WindowStatusBar::requestNotify);
    connect(NotificationManager::instance(), &NotificationManager::updated, this,
            [this] {
                d->notifyBtn->setIcon(QIcon::fromTheme("new_notification"));
            });

    addPermanentWidget(d->notifyBtn);
    this->addPermanentWidget(d->messageLabel);
    this->addPermanentWidget(d->progressBar);
    hideProgress();
}

WindowStatusBar::~WindowStatusBar()
{
    if (d) {
        delete d;
    }
}

void WindowStatusBar::setPercentage(int percentage)
{
    d->progressBar->setPercentage(percentage);
}

void WindowStatusBar::setMessage(const QString &message)
{
    d->messageLabel->setText(message);
    d->messageLabel->setToolTip(message);
}

void WindowStatusBar::hideProgress()
{
    d->progressBar->hide();
    d->messageLabel->hide();
}

void WindowStatusBar::showProgress()
{
    d->progressBar->show();
    d->messageLabel->show();
}

void WindowStatusBar::requestNotify()
{
    d->notifyBtn->setIcon(QIcon::fromTheme("notification"));
    NotificationManager::instance()->toggle();
}

bool WindowStatusBar::progressIsHidden()
{
    return d->progressBar->isHidden()
            && d->messageLabel->isHidden();
}

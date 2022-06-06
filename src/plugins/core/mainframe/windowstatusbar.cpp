/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "windowstatusbar.h"
#include "progressbar.h"
#include "common/common.h"

class WindowStatusBarPrivate
{
    friend class WindowStatusBar;
    ProgressBar *progressBar = nullptr;
    ElidedLabel *messageLabel = nullptr;
};

WindowStatusBar::WindowStatusBar(QWidget *parent)
    : QStatusBar(parent)
    , d(new WindowStatusBarPrivate())
{
    d->progressBar = new ProgressBar;
    d->progressBar->setFixedSize(80, 10);
    d->messageLabel = new ElidedLabel;
    d->messageLabel->setMaximumWidth(200);

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

bool WindowStatusBar::progressIsHidden()
{
    return d->progressBar->isHidden()
            && d->messageLabel->isHidden();
}

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
#include "processdialog.h"
#include "common/common.h"
#include <QLabel>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ProcessDialogPrivate
{
    friend class ProcessDialog;
    StatusWidget *statusWidget = nullptr;
    QTextBrowser *messageBrowser = nullptr;
    QHBoxLayout *hLayout = nullptr;
};

ProcessDialog::ProcessDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent, f)
    , d (new ProcessDialogPrivate)
{
    setAutoFillBackground(true);
    int lineHeight = this->fontMetrics().height();
    this->setMinimumWidth(500);
    this->setMinimumHeight(lineHeight * 2);
    d->statusWidget = new StatusWidget();
    d->statusWidget->setPatternFlags(StatusWidget::PatternFlag::Ring);
    d->statusWidget->setFixedSize(lineHeight * 2, lineHeight * 2);
    d->messageBrowser = new QTextBrowser();
    d->hLayout = new QHBoxLayout();

    this->setLayout(d->hLayout);
    d->hLayout->addWidget(d->statusWidget);
    d->hLayout->addWidget(d->messageBrowser);
}

ProcessDialog::~ProcessDialog()
{
    if (d) {
        delete d;
    }
}

ProcessDialog *ProcessDialog::globalInstance()
{
    static ProcessDialog dialog;
    return &dialog;
}

void ProcessDialog::setRunning(bool runable)
{
    if (runable)
        d->statusWidget->start();
    else
        d->statusWidget->stop();
}

ProcessDialog &ProcessDialog::operator <<(const QString &message)
{
    d->messageBrowser->append(message);
    return *this;
}

void ProcessDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "findtoolbar.h"
#include "transceiver/findsender.h"
#include "services/find/findservice.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#define LABEL_WIDTH           (80)
#define OPERATOR_WIDGET_WIDTH (400)
using namespace dpfservice;

class FindToolBarPrivate
{
    FindToolBarPrivate(){}
    QLineEdit *findLineEdit{nullptr};
    QLineEdit *replaceLineEdit{nullptr};

    friend class FindToolBar;
};

FindToolBar::FindToolBar(QWidget *parent)
    : QWidget(parent)
    , d(new FindToolBarPrivate())
{
    setupUi();
}

void FindToolBar::setupUi()
{
    setMaximumHeight(85);
    setMinimumWidth(800);

    QVBoxLayout *vLayout = new QVBoxLayout();
    QHBoxLayout *findLayout = new QHBoxLayout();
    vLayout->addLayout(findLayout);
    QLabel *findLabel = new QLabel(QLabel::tr("Find:"));
    findLabel->setFixedWidth(LABEL_WIDTH);
    findLabel->setAlignment(Qt::AlignRight);

    d->findLineEdit = new QLineEdit();

    QWidget *findWidget = new QWidget();
    findWidget->setFixedWidth(OPERATOR_WIDGET_WIDTH);
    QHBoxLayout *findWidgetLayout = new QHBoxLayout();
    findWidgetLayout->setMargin(0);
    findWidget->setLayout(findWidgetLayout);

    QPushButton *findPreBtn = new QPushButton(QPushButton::tr("Find Previous"));
    QPushButton *findNextBtn = new QPushButton(QPushButton::tr("Find Next"));
    QPushButton *advancedBtn = new QPushButton(QPushButton::tr("Advanced..."));
    findWidgetLayout->addWidget(findPreBtn);
    findWidgetLayout->addWidget(findNextBtn);
    findWidgetLayout->addWidget(advancedBtn);

    findLayout->addWidget(findLabel);
    findLayout->addWidget(d->findLineEdit);
    findLayout->addWidget(findWidget);

    QHBoxLayout *repalceLayout = new QHBoxLayout();
    vLayout->addLayout(repalceLayout);
    QLabel *repalceLabel = new QLabel(QLabel::tr("Repalce:"));
    repalceLabel->setFixedWidth(LABEL_WIDTH);
    repalceLabel->setAlignment(Qt::AlignRight);

    d->replaceLineEdit = new QLineEdit();

    QWidget *replaceWidget = new QWidget();
    replaceWidget->setFixedWidth(OPERATOR_WIDGET_WIDTH);
    QHBoxLayout *replaceWidgetLayout = new QHBoxLayout();
    replaceWidgetLayout->setMargin(0);
    replaceWidget->setLayout(replaceWidgetLayout);

    QPushButton *replaceBtn = new QPushButton(QPushButton::tr("Replace"));
    QPushButton *replaceFindBtn = new QPushButton(QPushButton::tr("Replace && Find"));
    QPushButton *replaceAllBtn = new QPushButton(QPushButton::tr("Repalce All"));
    replaceWidgetLayout->addWidget(replaceBtn);
    replaceWidgetLayout->addWidget(replaceFindBtn);
    replaceWidgetLayout->addWidget(replaceAllBtn);

    repalceLayout->addWidget(repalceLabel);
    repalceLayout->addWidget(d->replaceLineEdit);
    repalceLayout->addWidget(replaceWidget);

    connect(findPreBtn, &QAbstractButton::clicked, this, &FindToolBar::findPrevious);
    connect(findNextBtn, &QAbstractButton::clicked, this, &FindToolBar::findNext);
    connect(advancedBtn, &QAbstractButton::clicked, this, &FindToolBar::advancedSearch);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolBar::replace);
    connect(replaceFindBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceSearch);
    connect(replaceAllBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceAll);


    setLayout(vLayout);
}

void FindToolBar::findPrevious()
{
    QString text = d->findLineEdit->text();
    if (text.isEmpty())
        return;
    FindSender::sendSearchCommand(text, FindType::Previous);
}

void FindToolBar::findNext()
{
    QString text = d->findLineEdit->text();
    if (text.isEmpty())
        return;
    FindSender::sendSearchCommand(text, FindType::Next);
}

void FindToolBar::advancedSearch()
{
    emit advanced();
}

void FindToolBar::replace()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    FindSender::sendReplaceCommand(srcText, destText, RepalceType::Repalce);
}

void FindToolBar::replaceSearch()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    FindSender::sendReplaceCommand(srcText, destText, RepalceType::FindAndReplace);
}

void FindToolBar::replaceAll()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    FindSender::sendReplaceCommand(srcText, destText, RepalceType::RepalceAll);
}


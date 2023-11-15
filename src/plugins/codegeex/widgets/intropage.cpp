// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intropage.h"
#include "codegeexmanager.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFont>
#include <QDebug>

IntroPage::IntroPage(QWidget *parent)
    : QWidget (parent)
{
    initUI();
}

void IntroPage::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addSpacing(50);
    initLogo();
    mainLayout->addSpacing(50);
    initIntroContent();

    mainLayout->addStretch(1);
    initSuggestContent();
}

void IntroPage::initLogo()
{
    QVBoxLayout *logoLayout = new QVBoxLayout;
    logoLayout->setAlignment(Qt::AlignCenter);
    qobject_cast<QVBoxLayout*>(layout())->addLayout(logoLayout);

    QLabel *logo = new QLabel(this);
    logo->setAlignment(Qt::AlignHCenter);
    logo->setPixmap(QPixmap(":/resoures/images/logo-codegeex.png").scaledToWidth(80));
    logoLayout->addWidget(logo);

    QLabel *logoLabel = new QLabel(this);
    logoLabel->setAlignment(Qt::AlignHCenter);
    logoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto font = logoLabel->font();
    font.setPointSize(14);
    logoLabel->setFont(font);
    logoLabel->setText(tr("Welcome to CodeGeeX"));
    logoLayout->addWidget(logoLabel);
}

void IntroPage::initIntroContent()
{
    QVBoxLayout *introLayout = new QVBoxLayout;
    introLayout->setAlignment(Qt::AlignCenter);
    qobject_cast<QVBoxLayout*>(layout())->addLayout(introLayout);

    appendDescLabel(introLayout, tr("CodeGeeX provides code completion suggestions in editor, Press Tab to accept."));
    appendDescLabel(introLayout, tr("Select code and right-click to add comments or translate code."));
    appendDescLabel(introLayout, tr("Also, you can directly ask CodeGeeX any questions."));
}

void IntroPage::initSuggestContent()
{
    QVBoxLayout *suggestLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout*>(layout())->addLayout(suggestLayout);

    QLabel *suggestLabel = new QLabel(this);
    suggestLabel->setText(tr("Try the following questions:"));
    suggestLayout->addWidget(suggestLabel);

    appendSuggestButton(suggestLayout, tr("How to iterate through a dictionary in Python?"));
    appendSuggestButton(suggestLayout, tr("Write a quicksort function."));
    appendSuggestButton(suggestLayout, tr("What is FIFO?"));
    appendSuggestButton(suggestLayout, tr("What is the best way to start learning JavaScript?"));
}

void IntroPage::appendDescLabel(QVBoxLayout *layout, const QString &text)
{
    QLabel *descLabel = new QLabel(this);
    descLabel->setAlignment(Qt::AlignHCenter);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    descLabel->setWordWrap(true);
    descLabel->setText(text);
    layout->addWidget(descLabel);
}

void IntroPage::appendSuggestButton(QVBoxLayout *layout, const QString &text)
{
    QPushButton *suggestButton = new QPushButton(this);
    suggestButton->setFixedHeight(50);
    QFont buttonFont = suggestButton->font();
    buttonFont.setPointSize(12);
    suggestButton->setFont(buttonFont);
    suggestButton->setText(text);
    layout->addWidget(suggestButton);

    connect(suggestButton, &QPushButton::clicked, [ = ] {
        CodeGeeXManager::instance()->sendMessage(suggestButton->text());
    });
}

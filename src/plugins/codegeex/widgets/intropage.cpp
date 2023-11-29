// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intropage.h"
#include "codegeexmanager.h"

#include <DLabel>
#include <DPushButton>
#include <DScrollArea>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>

IntroPage::IntroPage(QWidget *parent)
    : DWidget (parent)
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
    logoLayout->setSpacing(20);
    qobject_cast<QVBoxLayout*>(layout())->addLayout(logoLayout);

    DLabel *logo = new DLabel(this);
    logo->setAlignment(Qt::AlignHCenter);
    logo->setPixmap(QIcon::fromTheme("codegeex_anwser_icon").pixmap(50));
    logoLayout->addWidget(logo);

    DLabel *logoLabel = new DLabel(this);
    logoLabel->setAlignment(Qt::AlignHCenter);
    logoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto font = logoLabel->font();
    font.setPointSize(11);
    logoLabel->setFont(font);
    logoLabel->setText(tr("Welcome to CodeGeeX"));
    logoLayout->addWidget(logoLabel);
}

void IntroPage::initIntroContent()
{
    QVBoxLayout *introLayout = new QVBoxLayout;
    introLayout->setAlignment(Qt::AlignCenter);
    introLayout->setSpacing(10);
    qobject_cast<QVBoxLayout*>(layout())->addLayout(introLayout);

    appendDescLabel(introLayout, tr("CodeGeeX provides code completion suggestions in editor, Press Tab to accept."));
    appendDescLabel(introLayout, tr("Select code and right-click to add comments or translate code."));
    appendDescLabel(introLayout, tr("Also, you can directly ask CodeGeeX any questions."));
}

void IntroPage::initSuggestContent()
{
    QVBoxLayout *suggestLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout*>(layout())->addLayout(suggestLayout);

    DLabel *suggestLabel = new DLabel(this);
    suggestLabel->setText(tr("Try the following questions:"));
    suggestLayout->addWidget(suggestLabel);

    appendSuggestButton(suggestLayout, tr("How to iterate through a dictionary in Python?"), "codegeex_comment");
    appendSuggestButton(suggestLayout, tr("Write a quicksort function."), "codegeex_function");
    appendSuggestButton(suggestLayout, tr("What is the best way to start learning JavaScript?"), "codegeex_advice");
}

void IntroPage::appendDescLabel(QVBoxLayout *layout, const QString &text)
{
    QHBoxLayout *descLayout = new QHBoxLayout;
    descLayout->setAlignment(Qt::AlignTop);

    DLabel *icon = new DLabel(this);
    icon->setMargin(2);
    icon->setAlignment(Qt::AlignTop);
    icon->setPixmap(QIcon::fromTheme("codegeex_indicate").pixmap(16));
    descLayout->addWidget(icon);

    DLabel *descLabel = new DLabel(this);
    descLabel->setAlignment(Qt::AlignLeft);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    descLabel->setWordWrap(true);
    descLabel->setText(text);
    descLayout->addWidget(descLabel);

    layout->addLayout(descLayout);
}

void IntroPage::appendSuggestButton(QVBoxLayout *layout, const QString &text, const QString &iconName)
{
    DPushButton *suggestButton = new DPushButton(this);
    suggestButton->setFixedHeight(36);

    suggestButton->setIcon(QIcon::fromTheme(iconName));
    suggestButton->setText(text);
    layout->addWidget(suggestButton);

    connect(suggestButton, &DPushButton::clicked, [ = ] {
        emit suggestionToSend(suggestButton->text());
    });
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intropage.h"
#include "chatmanager.h"

#include <DScrollArea>
#include <DCommandLinkButton>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>
#include <QPaintEvent>

DWIDGET_USE_NAMESPACE

SuggestButton::SuggestButton(const QString &text, const QString &iconName, QWidget *parent)
    : DPushButton(parent), originalText(text)
{
    setFixedHeight(36);
    setText(text);
    setToolTip(text);
    setIcon(QIcon::fromTheme(iconName));
    minimumWidth = minimumSizeHint().width();
    setStyleSheet("text-align: left");

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, [this]() { setStyleSheet("text-align: left"); });
}

void SuggestButton::paintEvent(QPaintEvent *event)
{
    QFontMetrics fm(font());
    if (event->rect().width() < minimumWidth) {
        auto gap = minimumWidth - event->rect().width();
        auto length = fm.horizontalAdvance(originalText) - gap;
        if (length < 0)
            return DPushButton::paintEvent(event);
        setText(fm.elidedText(text(), Qt::ElideRight, length));
    } else if (text() != originalText) {
        setText(originalText);
    }
    DPushButton::paintEvent(event);
}

IntroPage::IntroPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
}

void IntroPage::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addSpacing(30);
    initLogo();
    mainLayout->addSpacing(30);
    initIntroContent();

    mainLayout->addStretch(1);
    initSuggestContent();
    mainLayout->addStretch(1);
}

void IntroPage::initLogo()
{
    QVBoxLayout *logoLayout = new QVBoxLayout;
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setSpacing(20);
    qobject_cast<QVBoxLayout *>(layout())->addLayout(logoLayout);

//    DLabel *logo = new DLabel(this);
//    logo->setAlignment(Qt::AlignHCenter);
//    logo->setPixmap(QIcon::fromTheme("uc_chat_anwser_icon").pixmap(50));
//    logoLayout->addWidget(logo);
    logoLayout->addSpacing(70);

    DLabel *logoLabel = new DLabel(this);
    logoLabel->setAlignment(Qt::AlignHCenter);
    logoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto font = logoLabel->font();
    font.setPointSize(11);
    logoLabel->setFont(font);
    logoLabel->setText(tr("Welcome to Chat"));
    logoLayout->addWidget(logoLabel);
}

void IntroPage::initIntroContent()
{
    QVBoxLayout *introLayout = new QVBoxLayout;
    introLayout->setAlignment(Qt::AlignCenter);
    introLayout->setSpacing(10);
    qobject_cast<QVBoxLayout *>(layout())->addLayout(introLayout);

    appendDescLabel(introLayout, tr("Chat provides code completion suggestions in editor, Press %1 Tab %2 to accept.").arg("<font style='color:dodgerblue;'>", "</font>"));
    appendDescLabel(introLayout, tr("Chat provides inline chat functionality in editor, Press %1 Ctrl + T %2 to use it.").arg("<font style='color:dodgerblue;'>", "</font>"));
    appendDescLabel(introLayout, tr("Also, you can directly %1 ask Chat any questions %2.").arg("<font style='color:dodgerblue;'>", "</font>"));
}

void IntroPage::initSuggestContent()
{
    QVBoxLayout *suggestLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout *>(layout())->addLayout(suggestLayout);

    DLabel *suggestLabel = new DLabel(this);
    suggestLabel->setText(tr("Try the following questions:"));
    suggestLayout->addWidget(suggestLabel);

    appendSuggestButton(suggestLayout, tr("How to iterate through a dictionary in Python?"), "uc_chat_comment");
    appendSuggestButton(suggestLayout, tr("Write a quicksort function."), "uc_chat_function");
    appendSuggestButton(suggestLayout, tr("What is the best way to start learning JavaScript?"), "uc_chat_advice");
}

void IntroPage::appendDescLabel(QVBoxLayout *layout, const QString &text)
{
    QHBoxLayout *descLayout = new QHBoxLayout;
    descLayout->setAlignment(Qt::AlignTop);

    DLabel *icon = new DLabel(this);
    icon->setContentsMargins(2, 2, 2, 2);
    icon->setFixedSize(16, 16);

    labelToPaint.append(icon);
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
    SuggestButton *suggestButton = new SuggestButton(text, iconName, this);
    layout->addWidget(suggestButton);

    connect(suggestButton, &DPushButton::clicked, [=] {
        emit suggestionToSend(text);
    });
}

void IntroPage::paintEvent(QPaintEvent *event)
{
    auto pa = QPainter(this);
    pa.setPen(QColor(30, 144, 255));

    auto icon = QIcon::fromTheme("uc_chat_indicate");
    for (auto index = 0; index < labelToPaint.count(); index++) {
        auto rect = labelToPaint[index]->geometry();
        icon.paint(&pa, rect);
    }

    DWidget::paintEvent(event);
}

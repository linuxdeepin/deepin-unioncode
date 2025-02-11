// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatwidget.h"
#include "askpagewidget.h"
#include "historylistwidget.h"
#include "chatmanager.h"
#include "copilot.h"

#include <DLabel>
#include <DStackedWidget>
#include <DSuggestButton>

#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QResizeEvent>

ChatWidget::ChatWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
    initConnection();
}

void ChatWidget::onNewSessionCreated()
{
    stackWidget->setCurrentIndex(1);

    if (askPage)
        askPage->setIntroPage();
}

void ChatWidget::onCloseHistoryWidget()
{
    historyWidgetAnimation->setStartValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = false;
}

void ChatWidget::onShowHistoryWidget()
{
    if (!historyWidget || !historyWidgetAnimation)
        return;

    historyWidgetAnimation->setStartValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = true;
}

void ChatWidget::resizeEvent(QResizeEvent *event)
{
    if (historyWidget) {
        if (historyShowed) {
            historyWidget->setGeometry(0, 0, this->width(), this->height());
        } else {
            historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
        }
    }

    DWidget::resizeEvent(event);
}

void ChatWidget::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContentsMargins(0, 0, 0, 0);
    auto mainLayout = new QVBoxLayout(this);

    initAskWidget();
    initHistoryWidget();
    onNewSessionCreated(); // todo: modifed

//    auto initLoginUI = [this]() {
//        auto mainLayout = new QVBoxLayout(this);
//        auto loginWidget = new DWidget(this);
//        loginWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//        auto verticalLayout = new QVBoxLayout(loginWidget);
//        verticalLayout->setAlignment(Qt::AlignCenter);
//        verticalLayout->setContentsMargins(50, 0, 50, 50);

//        auto label_icon = new DLabel(this);
//        label_icon->setPixmap(QIcon::fromTheme("Chat_logo").pixmap(QSize(40, 26)));
//        label_icon->setAlignment(Qt::AlignCenter);

//        verticalLayout->addWidget(label_icon);

//        auto welcome_label = new DLabel(loginWidget);
//        welcome_label->setText(tr("Welcome to Chat"));//\nA must-have all-round AI tool for developers
//        welcome_label->setWordWrap(true);
//        welcome_label->setAlignment(Qt::AlignCenter);

//        auto font = welcome_label->font();
//        font.setPixelSize(14);
//        font.setWeight(500);
//        welcome_label->setFont(font);

//        auto descrption_label = new DLabel(loginWidget);
//        descrption_label->setText(tr("A must-have all-round AI tool for developers"));
//        descrption_label->setWordWrap(true);
//        descrption_label->setAlignment(Qt::AlignCenter);

//        font = descrption_label->font();
//        font.setPixelSize(12);
//        font.setWeight(400);
//        descrption_label->setFont(font);

//        verticalLayout->addSpacing(30);
//        verticalLayout->addWidget(welcome_label);
//        verticalLayout->addSpacing(5);
//        verticalLayout->addWidget(descrption_label);

//        auto btnLayout = new QHBoxLayout;     //make DSuggestBtn alignCenter
//        auto loginBtn = new DSuggestButton(loginWidget);
//        loginBtn->setText(tr("Go to login"));

//        btnLayout->addWidget(loginBtn, Qt::AlignHCenter);

//        verticalLayout->addSpacing(30);
//        verticalLayout->addLayout(btnLayout, Qt::AlignCenter);

//        mainLayout->addWidget(loginWidget, 1, Qt::AlignVCenter);
//    };
//    initLoginUI();
}

void ChatWidget::initConnection()
{
}

void ChatWidget::initAskWidget()
{
    stackWidget = new QStackedWidget(this);
    stackWidget->setContentsMargins(0, 0, 0, 0);
    stackWidget->setFrameShape(QFrame::NoFrame);
    stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    auto mainLayout = qobject_cast<QVBoxLayout *>(layout());
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(stackWidget);

    initStackWidget();
    initAskWidgetConnection();
}

void ChatWidget::initHistoryWidget()
{
    historyWidget = new HistoryListWidget(this);
    historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
    historyWidget->show();

    historyWidgetAnimation = new QPropertyAnimation(historyWidget, "geometry");
    historyWidgetAnimation->setEasingCurve(QEasingCurve::InOutSine);
    historyWidgetAnimation->setDuration(300);

    initHistoryWidgetConnection();
}

void ChatWidget::initStackWidget()
{
    askPage = new AskPageWidget(this);

    DWidget *creatingSessionWidget = new DWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    creatingSessionWidget->setLayout(layout);

    DLabel *creatingLabel = new DLabel(creatingSessionWidget);
    creatingLabel->setAlignment(Qt::AlignCenter);
    creatingLabel->setText(tr("Creating a new session..."));
    layout->addWidget(creatingLabel);

    stackWidget->insertWidget(0, creatingSessionWidget);
    stackWidget->insertWidget(1, askPage);
    stackWidget->setCurrentIndex(0);
}

void ChatWidget::initAskWidgetConnection()
{
    connect(askPage, &AskPageWidget::requestShowHistoryPage, this, &ChatWidget::onShowHistoryWidget);
}

void ChatWidget::initHistoryWidgetConnection()
{
    connect(historyWidget, &HistoryListWidget::requestCloseHistoryWidget, this, &ChatWidget::onCloseHistoryWidget);
}

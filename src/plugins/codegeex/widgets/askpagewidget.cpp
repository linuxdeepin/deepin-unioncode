// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "askpagewidget.h"
#include "intropage.h"
#include "messagecomponent.h"
#include "codegeexmanager.h"

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QScrollBar>

AskPageWidget::AskPageWidget(QWidget *parent)
    : QWidget (parent)
{
    initUI();
    initConnection();
}

void AskPageWidget::setIntroPage()
{
    cleanWidgets();
    curState = Intro;
    scrollArea->setWidget(new IntroPage(scrollArea));

    Q_EMIT introPageShown();
}

bool AskPageWidget::isIntroPageState()
{
    return curState == Intro;
}

void AskPageWidget::onMessageUpdate(const MessageData &msgData)
{
    if (curState == Intro) {
        curState = Session;
        setSessionPage();
    }

    if (!msgComponents.contains(msgData.messageID())) {
        msgComponents.insert(msgData.messageID(), new MessageComponent(msgData));
        qobject_cast<QVBoxLayout*>(messageContainer->layout())->insertWidget(msgComponents.count() - 1,msgComponents.value(msgData.messageID()));
        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    } else {
        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    }

    if (scrollArea->verticalScrollBar()->isVisible()) {
        int maxValue = scrollArea->verticalScrollBar()->maximum();
        scrollArea->verticalScrollBar()->setValue(maxValue);
    }
}

void AskPageWidget::onSendBtnClicked()
{
    if (inputEdit) {
        auto prompt = inputEdit->text();
        if (prompt.isEmpty())
            return;

        CodeGeeXManager::instance()->sendMessage(prompt);
        inputEdit->clear();
    }
}

void AskPageWidget::initUI()
{
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    scrollArea = new QScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter);
    layout->addWidget(scrollArea);

    inputWidget = new QWidget(this);
    layout->addWidget(inputWidget);

    initInputWidget();
}

void AskPageWidget::initInputWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    inputWidget->setLayout(layout);

    QHBoxLayout *editLayout = new QHBoxLayout;
    layout->addLayout(editLayout);

    inputEdit = new QLineEdit(inputWidget);
    inputEdit->setFixedHeight(50);
    inputEdit->setPlaceholderText(tr("Ask question here, press Enter to send..."));
    editLayout->addWidget(inputEdit);

//    QLabel *inputTips = new QLabel(inputWidget);
//    inputTips->setText(tr("Ctrl + Enter for Newline | \" / \" for command"));
//    inputTips->setIndent(10);
//    layout->addWidget(inputTips);
}

void AskPageWidget::initConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestMessageUpdate, this, &AskPageWidget::onMessageUpdate);
    connect(inputEdit, &QLineEdit::returnPressed, this, &AskPageWidget::onSendBtnClicked);
}

void AskPageWidget::cleanWidgets()
{
    if (auto currentWidget = scrollArea->takeWidget())
        currentWidget->deleteLater();
}

void AskPageWidget::setSessionPage()
{
    cleanWidgets();
    messageContainer = new QWidget(scrollArea);
    scrollArea->setWidget(messageContainer);

    QVBoxLayout *layout = new QVBoxLayout(messageContainer);
    messageContainer->setLayout(layout);

    layout->addStretch(0);

    Q_EMIT sessionPageShown();
}

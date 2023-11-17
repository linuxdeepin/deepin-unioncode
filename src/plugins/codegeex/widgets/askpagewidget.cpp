// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "askpagewidget.h"
#include "intropage.h"
#include "messagecomponent.h"
#include "codegeexmanager.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DScrollArea>
#include <DScrollBar>

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>

AskPageWidget::AskPageWidget(QWidget *parent)
    : DWidget (parent)
    , processTimer(new QTimer(this))
{
    initUI();
    initConnection();

    processTimer->setInterval(200);
}

void AskPageWidget::setIntroPage()
{
    cleanWidgets();
    curState = Intro;
    IntroPage *introPage = new IntroPage(scrollArea);
    connect(introPage, &IntroPage::suggestionToSend, [this](const QString &suggesstion){
        askQuestion(suggesstion);
    });
    scrollArea->setWidget(introPage);

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
        msgComponents.insert(msgData.messageID(), new MessageComponent(msgData, messageContainer));
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

        askQuestion(prompt);
    }
}

void AskPageWidget::onChatFinished()
{
    enterInputState();
}

void AskPageWidget::initUI()
{
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    scrollArea = new DScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter);
    layout->addWidget(scrollArea);

    inputWidget = new DWidget(this);
    layout->addWidget(inputWidget);

    initInputWidget();
}

void AskPageWidget::initInputWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    inputWidget->setLayout(layout);

    inputEdit = new DLineEdit(inputWidget);
    inputEdit->setFixedHeight(50);
    placeHolderText = tr("Ask question here, press Enter to send...");
    inputEdit->setPlaceholderText(placeHolderText);
    layout->addWidget(inputEdit);
}

void AskPageWidget::initConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestMessageUpdate, this, &AskPageWidget::onMessageUpdate);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatFinished, this, &AskPageWidget::onChatFinished);
    connect(processTimer, &QTimer::timeout, [this](){
        QString tips;
        int maxDotNum = 4;
        int dotNum = progressCalcNum++ % maxDotNum;
        for (int i = 0; i < dotNum; i++) {
            tips += "...";
        }
        QString holderText = tr("Answering") + tips;
        inputEdit->setPlaceholderText(holderText);
    });
    connect(inputEdit, &DLineEdit::returnPressed, this, &AskPageWidget::onSendBtnClicked);
}

void AskPageWidget::cleanWidgets()
{
    if (auto currentWidget = scrollArea->takeWidget()) {
        currentWidget->deleteLater();
        msgComponents.clear();
    }
}

void AskPageWidget::setSessionPage()
{
    cleanWidgets();
    messageContainer = new DWidget(scrollArea);
    scrollArea->setWidget(messageContainer);

    QVBoxLayout *layout = new QVBoxLayout(messageContainer);
    messageContainer->setLayout(layout);

    layout->addStretch(1);

    Q_EMIT sessionPageShown();
}

void AskPageWidget::enterAnswerState()
{
    progressCalcNum = 0;
    inputEdit->clear();
    inputEdit->setEnabled(false);
    processTimer->start();
}

void AskPageWidget::enterInputState()
{
    processTimer->stop();
    inputEdit->setEnabled(true);
    inputEdit->setPlaceholderText(placeHolderText);
}

void AskPageWidget::askQuestion(const QString &question)
{
    CodeGeeXManager::instance()->sendMessage(question);
    enterAnswerState();
}

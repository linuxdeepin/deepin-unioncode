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
#include <DHorizontalLine>

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

    resetBtns();

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

    QTimer::singleShot(100, [this](){
        if (scrollArea->verticalScrollBar()->isVisible()) {
            int maxValue = scrollArea->verticalScrollBar()->maximum();
            scrollArea->verticalScrollBar()->setValue(maxValue);
        }
    });
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

void AskPageWidget::onDeleteBtnClicked()
{
    CodeGeeXManager::instance()->deleteCurrentSession();
    CodeGeeXManager::instance()->cleanHistoryMessage();
}

void AskPageWidget::onHistoryBtnClicked()
{
    Q_EMIT requestShowHistoryPage();
}

void AskPageWidget::onCreateNewBtnClicked()
{
    CodeGeeXManager::instance()->cleanHistoryMessage();
    CodeGeeXManager::instance()->createNewSession();
}

void AskPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    scrollArea = new DScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter);
    layout->addWidget(scrollArea);

    DHorizontalLine *line = new DHorizontalLine(this);
    layout->addWidget(line);

    inputWidget = new DWidget(this);
    layout->addWidget(inputWidget);

    initInputWidget();
}

void AskPageWidget::initInputWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    inputWidget->setLayout(layout);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    layout->addLayout(btnLayout);

    deleteBtn = new DPushButton(this);
    deleteBtn->setFlat(true);
    deleteBtn->setIcon(QIcon::fromTheme("codegeex_clear"));
    btnLayout->addWidget(deleteBtn);

    btnLayout->addStretch(1);

    historyBtn = new DPushButton(this);
    historyBtn->setFlat(true);
    historyBtn->setIcon(QIcon::fromTheme("codegeex_history"));
    btnLayout->addWidget(historyBtn);
    createNewBtn = new DPushButton(this);
    createNewBtn->setFlat(true);
    createNewBtn->setIcon(QIcon::fromTheme("codegeex_new"));
    btnLayout->addWidget(createNewBtn);

    auto hlayout = new QHBoxLayout;
    inputEdit = new DLineEdit(inputWidget);
    inputEdit->setFixedHeight(50);
    placeHolderText = tr("Ask question here, press Enter to send...");
    inputEdit->setPlaceholderText(placeHolderText);
    sendButton = new DFloatingButton(this);
    sendButton->setFixedSize(30, 30);
    sendButton->setIcon(QIcon::fromTheme("codegeex_send").pixmap(16));
    sendButton->setEnabled(false);

    hlayout->addWidget(inputEdit);
    hlayout->setSpacing(10);
    hlayout->addWidget(sendButton);

    layout->addLayout(hlayout);
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

    connect(sendButton, &DFloatingButton::clicked, inputEdit, &DLineEdit::returnPressed);
    connect(inputEdit, &DLineEdit::returnPressed, this, &AskPageWidget::onSendBtnClicked);
    connect(deleteBtn, &DPushButton::clicked, this, &AskPageWidget::onDeleteBtnClicked);
    connect(historyBtn, &DPushButton::clicked, this, &AskPageWidget::onHistoryBtnClicked);
    connect(createNewBtn, &DPushButton::clicked, this, &AskPageWidget::onCreateNewBtnClicked);
    connect(inputEdit, &DLineEdit::textChanged, sendButton, [this](){
        if(inputEdit->text().isEmpty())
            sendButton->setEnabled(false);
        else
            sendButton->setEnabled(true);
    });
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
    layout->setSpacing(10);
    messageContainer->setLayout(layout);

    layout->addStretch(1);

    resetBtns();

    Q_EMIT sessionPageShown();
}

void AskPageWidget::enterAnswerState()
{
    progressCalcNum = 0;
    inputEdit->clear();
    inputEdit->setEnabled(false);

    if (deleteBtn)
        deleteBtn->setEnabled(false);
    if (historyBtn)
        historyBtn->setEnabled(false);
    if (createNewBtn)
        createNewBtn->setEnabled(false);

    processTimer->start();
}

void AskPageWidget::enterInputState()
{
    processTimer->stop();
    inputEdit->setEnabled(true);
    inputEdit->setPlaceholderText(placeHolderText);

    if (deleteBtn)
        deleteBtn->setEnabled(true);
    if (historyBtn)
        historyBtn->setEnabled(true);
    if (createNewBtn)
        createNewBtn->setEnabled(true);
}

void AskPageWidget::askQuestion(const QString &question)
{
    CodeGeeXManager::instance()->sendMessage(question);
    enterAnswerState();
}

void AskPageWidget::resetBtns()
{
    if (!deleteBtn || !historyBtn || !createNewBtn)
        return;

    deleteBtn->setEnabled(!isIntroPageState());
    createNewBtn->setVisible(!isIntroPageState());
    historyBtn->setVisible(true);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagecomponent.h"
#include "codeeditcomponent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QPalette>
#include <QFrame>
#include <QDebug>
#include <QRegularExpression>

MessageComponent::MessageComponent(const MessageData &msgData, QWidget *parent)
    : QWidget(parent),
      messageData(msgData)
{
    initUI();
}
void MessageComponent::updateMessage(const MessageData &msgData)
{
    if (currentUpdateState == CodeEdit && msgData.messageLines().last() == "```") {
        curUpdateEdit->cleanFinalLine();
        currentUpdateState = Label;
        messageData = msgData;
        return;
    } else if (currentUpdateState == Label && msgData.messageLines().last().startsWith("```")) {
        if (curUpdateLabel->text() == "``" | curUpdateLabel->text() == "`") {
            msgLayout->removeWidget(curUpdateLabel);
            delete curUpdateLabel;
            curUpdateLabel = nullptr;
        }
        currentUpdateState = CodeEdit;
        curUpdateEdit = new CodeEditComponent(this);
        curUpdateEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        curUpdateEdit->setReadOnly(true);
        curUpdateEdit->setUpdateHeight(true);
        curUpdateEdit->showButtons(CodeEditComponent::CopyAndInsert);
        msgLayout->addWidget(curUpdateEdit);
    }

    switch (currentUpdateState) {
    case Label:
        if (!curUpdateLabel) {
            curUpdateLabel = new QLabel(this);
            curUpdateLabel->setWordWrap(true);
            msgLayout->addWidget(curUpdateLabel);
        } else if (msgData.messageLines().length() > messageData.messageLines().length()) {
            curUpdateLabel = new QLabel(this);
            curUpdateLabel->setWordWrap(true);
            msgLayout->addWidget(curUpdateLabel);
        }
        curUpdateLabel->setText(msgData.messageLines().last());
        break;
    case CodeEdit:
        if (msgData.messageLines().at(msgData.messageLines().length() - 2) == "```") {
            curUpdateEdit->cleanFinalLine();
            currentUpdateState = Label;
            updateMessage(msgData);
            return;
        }
        if (curUpdateEdit) {
            int startIndex = msgData.messageLines().lastIndexOf(QRegularExpression("```([a-z]+|[A-Z]+)"));
            curUpdateEdit->updateCode(msgData.messageLines().mid(startIndex + 1));
        }
        break;
    }
    messageData = msgData;
}

void MessageComponent::initUI()
{
    QVBoxLayout *msgLayout = new QVBoxLayout;
    setLayout(msgLayout);

//    if (messageData.messageType() == MessageData::Ask) {
//        auto palatte = palette();
//        auto windowColor = palatte.color(QPalette::Normal, QPalette::Window);
//        windowColor.setRgb(windowColor.red() + 10, windowColor.green() + 10, windowColor.blue() + 10);
//        palatte.setColor(QPalette::Window, Qt::white);
//        setPalette(palatte);
//        setAutoFillBackground(true);
//    }

    initSenderInfo();
    initMessageSection();
}

void MessageComponent::initSenderInfo()
{
    QHBoxLayout *senderInfoLayout = new QHBoxLayout;
    qobject_cast<QVBoxLayout*>(layout())->addLayout(senderInfoLayout);

    senderHead = new QLabel(this);
    senderName = new QLabel(this);

    QImage *headImg = new QImage(QSize(40, 40), QImage::Format_ARGB32);
    switch (messageData.messageType()) {
    case MessageData::Ask:
        headImg->fill(Qt::red);
        senderName->setText("You");
        break;
    case MessageData::Anwser:
        headImg->fill(Qt::blue);
        senderName->setText("CodeGeeX");
        break;
    }
    senderHead->setPixmap(QPixmap::fromImage(*headImg));

    senderInfoLayout->addWidget(senderHead);
    senderInfoLayout->addWidget(senderName);
    senderInfoLayout->addStretch(1);
}

void MessageComponent::initMessageSection()
{
    msgLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout*>(layout())->addLayout(msgLayout);
}

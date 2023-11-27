// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagecomponent.h"
#include "codeeditcomponent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <DLabel>
#include <QPalette>
#include <QFrame>
#include <QDebug>
#include <QRegularExpression>

MessageComponent::MessageComponent(const MessageData &msgData, QWidget *parent)
    : DWidget(parent),
      messageData(msgData)
{
    initUI();
}
void MessageComponent::updateMessage(const MessageData &msgData)
{
    if (msgData.messageType() == MessageData::Ask) {
        curUpdateLabel = new DLabel(this);
        curUpdateLabel->setWordWrap(true);
        curUpdateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        msgLayout->addWidget(curUpdateLabel);
        curUpdateLabel->setText(msgData.messageData());
        return;
    }

    if (!createCodeEdit(msgData))
        return;

    switch (currentUpdateState) {
    case Label:
        if (!curUpdateLabel) {
            curUpdateLabel = new DLabel(this);
            curUpdateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            curUpdateLabel->setWordWrap(true);
            msgLayout->addWidget(curUpdateLabel);
        } else if (msgData.messageLines().length() > messageData.messageLines().length()) {
            curUpdateLabel = new DLabel(this);
            curUpdateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            curUpdateLabel->setWordWrap(true);
            msgLayout->addWidget(curUpdateLabel);
        }
        curUpdateLabel->setText(msgData.messageLines().last());
        break;
    case CodeEdit:
        if (curUpdateEdit) {
            int startIndex = msgData.messageLines().lastIndexOf(QRegularExpression("```([a-z]*|[A-Z]*)"));
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

    senderHead = new DLabel(this);
    senderName = new DLabel(this);

    switch (messageData.messageType()) {
    case MessageData::Ask: {
        senderName->setText("You");
        senderHead->setPixmap(QIcon::fromTheme("codegeex_user").pixmap(30, 30));
        break;
    }
    case MessageData::Anwser:
        senderName->setText("CodeGeeX");
        senderHead->setPixmap(QIcon::fromTheme("codegeex_anwser_icon").pixmap(30, 30));
        break;
    }

    senderInfoLayout->setSpacing(5);
    senderInfoLayout->addWidget(senderHead);
    senderInfoLayout->addWidget(senderName);
    senderInfoLayout->addStretch(1);
}

void MessageComponent::initMessageSection()
{
    msgLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout*>(layout())->addLayout(msgLayout);
}

bool MessageComponent::createCodeEdit(const MessageData &newData)
{
    QStringList newLines = newData.messageLines();
    QStringList oldLines = messageData.messageLines();
    QStringList addedLines = newLines.mid(oldLines.count());

    for (int i = 0; i < addedLines.count(); ++i) {
        QString addedLine = addedLines.at(i);
        if (addedLine.contains("`")) {
            if (i != 0) {
                MessageData addedMsgData = messageData;
                addedMsgData.appendData(addedLines.mid(0, i));
                updateMessage(addedMsgData);
            }

            QRegExp re("```([a-z]*|[A-Z]*)");
            if (re.exactMatch(addedLine) && currentUpdateState == Label) {
                // create new code edit component
                messageData.appendData({ addedLine });
                currentUpdateState = CodeEdit;
                curUpdateEdit = new CodeEditComponent(this);
                curUpdateEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
                curUpdateEdit->setReadOnly(true);
                curUpdateEdit->setUpdateHeight(true);
                curUpdateEdit->showButtons(CodeEditComponent::CopyAndInsert);
                msgLayout->addWidget(curUpdateEdit);
                return true;
            } else if (addedLine == "```" && currentUpdateState == CodeEdit) {
                // end the code edit component update
                messageData.appendData({ addedLine });
                currentUpdateState = Label;
                curUpdateLabel = new DLabel(this);
                curUpdateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                curUpdateLabel->setWordWrap(true);
                msgLayout->addWidget(curUpdateLabel);
                if (i != (addedLines.count() - 1))
                    updateMessage(newData);

                return false;
            }

            return false;
        }
    }

    return true;
}

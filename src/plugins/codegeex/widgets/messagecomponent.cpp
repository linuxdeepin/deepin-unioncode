// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagecomponent.h"
#include "codeeditcomponent.h"
#include "codegeexmanager.h"

#include <DLabel>
#include <DIconTheme>
#include <DPushButton>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBitmap>
#include <QPixmap>
#include <QPalette>
#include <QFrame>
#include <QDebug>
#include <QRegularExpression>

MessageComponent::MessageComponent(const MessageData &msgData, QWidget *parent)
    : DFrame(parent),
      messageData(msgData)
{
    initUI();
}

void MessageComponent::updateMessage(const MessageData &msgData)
{
    stopWaiting();
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
        if (!messageData.messageLines().isEmpty() && msgData.messageLines().last()
                != messageData.messageLines().last()) {
            auto messageLine = msgData.messageLines().last();
            // TODO(Mozart): use markdown format
            messageLine.replace("`", "");
            curUpdateLabel->setText(messageLine);
        }
        break;
    case CodeEdit:
        if (curUpdateEdit) {
            int startIndex = msgData.messageLines().lastIndexOf(QRegularExpression("```([a-z]*|[A-Z]*)"));
            if (startIndex != -1)
                curUpdateEdit->updateCode(msgData.messageLines().mid(startIndex + 1), msgData.messageLines().mid(startIndex, 1).at(0));
        }
        break;
    }
    messageData = msgData;
}

void MessageComponent::initUI()
{
    setAutoFillBackground(true);
    setLineWidth(0);

    QVBoxLayout *msgLayout = new QVBoxLayout;
    setLayout(msgLayout);

    QColor bgColor;
    if (messageData.messageType() == MessageData::Ask) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette palatte = DPaletteHelper::instance()->palette(this);
        bgColor = palatte.color(DPalette::ColorType::LightLively);
        bgColor.setAlpha(0xD);
        palatte.setColor(DPalette::ColorType::LightLively, bgColor);
        DPaletteHelper::instance()->setPalette(this, palatte);
        setBackgroundRole(DPalette::ColorType::LightLively);
#else
        auto palatte = palette();
        bgColor = palatte.color(QPalette::Highlight);
        bgColor.setAlpha(0xD);
        palatte.setColor(QPalette::Background, bgColor);
        setPalette(palatte);
        setBackgroundRole(QPalette::Background);
#endif
    } else {
        setBackgroundRole(DPalette::ColorType::ItemBackground);
    }

    initSenderInfo();
    initMessageSection();
    initConnect();
}

void MessageComponent::initSenderInfo()
{
    QHBoxLayout *senderInfoLayout = new QHBoxLayout;
    qobject_cast<QVBoxLayout *>(layout())->addLayout(senderInfoLayout);

    senderHead = new DLabel(this);
    senderName = new DLabel(this);

    editButton = new DPushButton(this);
    editButton->setIcon(DIconTheme::findQIcon("codegeex_edit"));
    editButton->setFlat(true);
    editButton->setFixedSize(QSize(24, 24));
    editButton->setToolTip(tr("edit"));

    switch (messageData.messageType()) {
    case MessageData::Ask: {
        senderName->setText("You");
        senderHead->setPixmap(DIconTheme::findQIcon("codegeex_user").pixmap(24, 24));
        break;
    }
    case MessageData::Anwser:
        senderName->setText("CodeGeeX");
        senderHead->setPixmap(DIconTheme::findQIcon("codegeex_anwser_icon").pixmap(24, 24));
        editButton->setVisible(false);
        break;
    }

    senderInfoLayout->setSpacing(5);
    senderInfoLayout->addWidget(senderHead);
    senderInfoLayout->addWidget(senderName);
    senderInfoLayout->addStretch(1);
    senderInfoLayout->addWidget(editButton);
}

void MessageComponent::initMessageSection()
{
    msgLayout = new QVBoxLayout;
    qobject_cast<QVBoxLayout *>(layout())->addLayout(msgLayout);
}

void MessageComponent::initConnect()
{
    if(!editButton)
        return;
    connect(editButton, &QPushButton::clicked, this, [=](){
        CodeGeeXManager::instance()->setMessage(messageData.messageData());
    });
}

void MessageComponent::waitForAnswer()
{
    waitingAnswer = true;
    auto hlayout = new QHBoxLayout;
    spinner = new DSpinner(this);
    spinner->setFixedSize(14, 14);
    hlayout->addWidget(spinner);
    hlayout->setAlignment(Qt::AlignLeft);

    msgLayout->addLayout(hlayout);
    spinner->start();
}

void MessageComponent::stopWaiting()
{
    if (waitingAnswer) {
        msgLayout->removeWidget(spinner);
        delete spinner;
        waitingAnswer = false;
    }
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

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagecomponent.h"
#include "codeeditcomponent.h"

#include <DLabel>
#include <DPushButton>
#include <DListView>
#include <DCommandLinkButton>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QDesktopServices>
#include <QStringListModel>
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
        if (!messageData.messageLines().isEmpty() && msgData.messageLines().last() != messageData.messageLines().last()) {
            auto messageLine = msgData.messageLines().last();
            // TODO(Mozart): use markdown format
            messageLine.replace("`", "");
            if (isConnecting && messageLine.contains("citation")) {
                QRegularExpression regex("\\[\\[citation:(\\d+)\\]\\]");
                messageLine = messageLine.replace(regex, "[\\1]");
            }
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
    editButton->setIcon(QIcon::fromTheme("codegeex_edit"));
    editButton->setFlat(true);
    editButton->setFixedSize(QSize(24, 24));
    editButton->setToolTip(tr("edit"));

    switch (messageData.messageType()) {
    case MessageData::Ask: {
        senderName->setText("You");
        senderHead->setPixmap(QIcon::fromTheme("codegeex_user").pixmap(24, 24));
        break;
    }
    case MessageData::Anwser:
        senderName->setText("CodeGeeX");
        senderHead->setPixmap(QIcon::fromTheme("codegeex_anwser_icon").pixmap(24, 24));
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
    if (!editButton)
        return;
    connect(editButton, &QPushButton::clicked, this, [=]() {
        CodeGeeXManager::instance()->setMessage(messageData.messageData());
    });
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::crawledWebsite, this,
            [=](const QString &msgID, const QList<CodeGeeX::websiteReference> &websites) {
                Q_UNUSED(msgID);
                if (!finished)
                    this->websites = websites;
            });
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatFinished, this,
            [=]() {
                if (isConnecting)
                    showWebsitesRefrences();
                finished = true;
            });
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::terminated, this,
            [=]() {
                finished = true;
            });
}

void MessageComponent::waitForAnswer()
{
    waitingAnswer = true;
    auto spinner = new DSpinner(this);
    spinner->setFixedSize(14, 14);

    searchingWidget = new DWidget(this);
    auto hlayout = new QHBoxLayout(searchingWidget);
    auto searchingIcon = new DLabel(searchingWidget);
    searchingIcon->setPixmap(QIcon::fromTheme("codegeex_internet").pixmap(QSize(14, 14)));
    auto searchingText = new DLabel(tr("Online Searching"), searchingWidget);

    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setAlignment(Qt::AlignLeft);
    hlayout->addWidget(searchingIcon);
    hlayout->addWidget(searchingText);
    hlayout->addWidget(spinner);
    msgLayout->addWidget(searchingWidget);
    isConnecting = CodeGeeXManager::instance()->isConnectToNetWork();
    if (!isConnecting) {
        searchingIcon->hide();
        searchingText->hide();
    }
    spinner->start();
}

void MessageComponent::stopWaiting()
{
    if (waitingAnswer) {
        msgLayout->removeWidget(searchingWidget);
        searchingWidget->hide();
        delete searchingWidget;
        waitingAnswer = false;
    }
}

void MessageComponent::setCustomWidget(QWidget *widget)
{
    if (widget && !widget->parent())
        widget->setParent(this);

    msgLayout->addWidget(widget);
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
            } else if (addedLine.size() > 4) {  // addedline starts with ` but not ```. eg: `123`
                return true;
            }
            return false;
        }
    }

    return true;
}

void MessageComponent::showWebsitesRefrences()
{
    if (finished)
        return;

    auto separator = new QHBoxLayout;
    separator->setContentsMargins(0, 0, 0, 0);
    auto toggleBtn = new DPushButton(this);
    toggleBtn->setText(tr("Show Reference"));
    toggleBtn->setFlat(true);
    toggleBtn->setIcon(QIcon::fromTheme("codegeex_internet"));

    separator->addWidget(toggleBtn);
    msgLayout->addLayout(separator);

    int count = 0;
    DListView *view = new DListView(this);
    view->setItemSpacing(2);
    view->setSelectionMode(DListView::NoSelection);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto listModel = new QStringListModel(this);
    QStringList stringList;
    view->setModel(listModel);
    for (auto website : websites) {
        QString title = QString::number(++count) + "." + website.title;
        stringList.append(title);
    }
    listModel->setStringList(stringList);
    msgLayout->addWidget(view);

    connect(view, &DListView::clicked, this, [=](const QModelIndex &index) {
        auto website = websites[index.row()];
        if (!QDesktopServices::openUrl(website.url))
            qWarning() << "can not open url: " << website.url;
    });
    connect(toggleBtn, &DPushButton::clicked, this, [=]() {
        if (view->isVisible()) {
            msgLayout->removeWidget(view);
            view->hide();
        } else {
            view->show();
            msgLayout->addWidget(view);
        }
    });
}

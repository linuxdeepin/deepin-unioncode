// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagecomponent.h"
#include "codeeditcomponent.h"
#include "common/util/eventdefinitions.h"

#include <cmark.h>

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
#include <QJsonArray>

MessageComponent::MessageComponent(const MessageData &msgData, QWidget *parent)
    : DFrame(parent),
      messageData(msgData)
{
    initUI();
}

/*
 Due to the use of multiple DLabels to display text in this module,
 the ordered list is always parsed starting from 1.
 To avoid this issue, we have implemented some circumvention measures.
 */
QString convertOlToParagraph(const QString &input)
{
    QString result = input;

    static QRegularExpression olRegex(
            R"(<ol(?:\s+start="(\d+)\")?\s*>\s*<li>(.*?)</li>\s*</ol>)",
            QRegularExpression::DotMatchesEverythingOption
            );

    QRegularExpressionMatch match = olRegex.match(input);
    if (match.hasMatch()) {
        QString startNum = match.captured(1);
        QString content = match.captured(2);

        QString replacement;
        if (startNum.isEmpty()) {
            replacement = QString("<p>1. %1</p>").arg(content);
        } else {
            replacement = QString("<p>%1. %2</p>").arg(startNum).arg(content);
        }

        result = result.replace(match.captured(0), replacement);
    }

    return result;
}

void MessageComponent::updateMessage(const MessageData &msgData)
{
    stopWaiting();
    if (msgData.messageType() == MessageData::Ask) {
        curUpdateLabel = new DLabel(this);
        curUpdateLabel->setWordWrap(true);
        curUpdateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        msgLayout->addWidget(curUpdateLabel);
        auto originText = msgData.messageData().toStdString();
        QString convertedHtml = cmark_markdown_to_html(originText.c_str(), originText.size(), CMARK_OPT_SMART);
        curUpdateLabel->setText(convertedHtml);
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
            if (isConnecting && messageLine.contains("citation")) {
                QRegularExpression regex("\\[\\[citation:(\\d+)\\]\\]");
                messageLine = messageLine.replace(regex, "[\\1]");
            }
            auto originText = messageLine.toStdString();
            QString convertedHtml = cmark_markdown_to_html(originText.c_str(), originText.size(), CMARK_OPT_SMART);
            if (convertedHtml.contains("<ol"))
                convertedHtml = convertOlToParagraph(convertedHtml);
            curUpdateLabel->setText(convertedHtml);
        } else if (messageData.messageLines().isEmpty()) {
            auto originText = msgData.messageData().toStdString();
            auto convertedHtml = cmark_markdown_to_html(originText.c_str(), originText.size(), CMARK_OPT_SMART);
            curUpdateLabel->setText(convertedHtml);
        }
        break;
    case CodeEdit:
        if (curUpdateEdit) {
            int startIndex = msgData.messageLines().lastIndexOf(QRegularExpression(R"(\s*```(`|[a-z]*|[A-Z]*))")); // ```` ```python ```cpp
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
            [=](const QString &msgID, const QList<websiteReference> &websites) {
                Q_UNUSED(msgID);
                if (!finished)
                    this->websites = websites;
            });
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatFinished, this,
            [=]() {
                if (isConnecting && !websites.isEmpty())
                    showWebsitesReferences();
                if (CodeGeeXManager::instance()->isReferenceCodebase())
                    showChunksReferences();
                finished = true;
            });
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestStop, this,
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
        QString addedLine = addedLines.at(i).trimmed();
        if (addedLine.contains("`") || addedLine.isEmpty()) {
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

void MessageComponent::showWebsitesReferences()
{
    if (finished || messageData.messageType() == MessageData::Ask)
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

void MessageComponent::showChunksReferences()
{
    if (finished || messageData.messageType() == MessageData::Ask)
        return;

    QJsonArray chunks = CodeGeeXManager::instance()->getCurrentChunks()["Chunks"].toArray();
    if (chunks.isEmpty())
        return;

    auto separator = new QHBoxLayout;
    separator->setContentsMargins(0, 0, 0, 0);
    auto toggleBtn = new DPushButton(this);
    toggleBtn->setText(tr("Show Reference"));
    toggleBtn->setFlat(true);
    toggleBtn->setIcon(QIcon::fromTheme("uc_codegeex_project_chat"));
    separator->addWidget(toggleBtn);
    msgLayout->addLayout(separator);

    DListView *view = new DListView(this);
    view->setItemSpacing(2);
    view->setSelectionMode(DListView::NoSelection);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setEditTriggers(DListView::EditTrigger::NoEditTriggers);
    view->setTextElideMode(Qt::ElideLeft);
    auto listModel = new QStringListModel(this);
    QStringList stringList;
    view->setModel(listModel);

    for (auto chunk : chunks) {
        QString title = chunk.toObject()["fileName"].toString();
        stringList.append(title);
        view->addItem(title);
    }
    listModel->setStringList(stringList);
    msgLayout->addWidget(view);

    connect(view, &DListView::doubleClicked, this, [=](const QModelIndex &index) {
        auto codeRange = index.data().toString();
        QRegularExpression re("(.+)\\[(\\d+)-\\d+\\]");
        QRegularExpressionMatch match = re.match(codeRange);

        if (match.hasMatch()) {
            QString filePath = match.captured(1);
            int startLine = match.captured(2).toInt();
            editor.gotoLine(filePath, startLine);
        }
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

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionrecorditem.h"

#include <DPushButton>

#include <QHBoxLayout>

SessionRecordItem::SessionRecordItem(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnection();
}

void SessionRecordItem::updateItem(const RecordData &data)
{
    talkId = data.talkId;

    if (promotLabel)
        promotLabel->setText(data.promot);

    if (dateLabel)
        dateLabel->setText(data.date);
}

void SessionRecordItem::onDeleteButtonClicked()
{
    CodeGeeXManager::instance()->deleteSession(talkId);
}

void SessionRecordItem::onRecordClicked()
{
    CodeGeeXManager::instance()->sendMessage(promotLabel->text());
}

void SessionRecordItem::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);

    recordButton = new DPushButton(this);
    recordButton->setFixedHeight(80);
    layout->addWidget(recordButton);

    QVBoxLayout *btnTextLayout = new QVBoxLayout;
    btnTextLayout->setMargin(5);
    btnTextLayout->setSpacing(20);

    promotLabel = new DLabel(recordButton);
    btnTextLayout->addWidget(promotLabel);

    dateLabel = new DLabel(recordButton);
    btnTextLayout->addWidget(dateLabel);

    recordButton->setLayout(btnTextLayout);

    deleteButton = new DPushButton(this);
    deleteButton->setFixedWidth(80);
    deleteButton->setText(tr("Delete"));
    layout->addWidget(deleteButton, 0);
}

void SessionRecordItem::initConnection()
{
    connect(recordButton, &DPushButton::clicked, this, &SessionRecordItem::onRecordClicked);
    connect(deleteButton, &DPushButton::clicked, this, &SessionRecordItem::onDeleteButtonClicked);
}

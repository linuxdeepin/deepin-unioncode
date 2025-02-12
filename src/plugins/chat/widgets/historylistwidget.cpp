// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "historylistwidget.h"
#include "pagecontrolcomponent.h"
#include "sessionrecorditem.h"

#include <DLabel>
#include <DPalette>
#include <DPageIndicator>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QResizeEvent>
#include <QFont>
#include <QtMath>

inline constexpr int kMaxItemCountInOnePage { 8 };

HistoryListWidget::HistoryListWidget(QWidget *parent)
    : DWidget (parent)
{
//    DPalette pal = palette();
//    pal.setColor(QPalette::Window, Qt::red);
//    setPalette(pal);
    setAutoFillBackground(true);

    initUI();
    initConnection();
}

void HistoryListWidget::onSessionRecordUpdated()
{
    auto records = ChatManager::instance()->sessionRecords();
    int pageCount =  qCeil(static_cast<qreal>(records.count()) / kMaxItemCountInOnePage);

    pageController->updatePageControl(pageCount);
    int currentPageIndex = pageController->currentIndex();

    int startRecordIndex = currentPageIndex * kMaxItemCountInOnePage;
    updateItems(records.mid(startRecordIndex, kMaxItemCountInOnePage));
}

void HistoryListWidget::onPageChanged(int index)
{
    auto records = ChatManager::instance()->sessionRecords();

    int startRecordIndex = index * kMaxItemCountInOnePage;
    updateItems(records.mid(startRecordIndex, kMaxItemCountInOnePage));
}

void HistoryListWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(10);
    setLayout(mainLayout);

    QHBoxLayout *headerLayout = new QHBoxLayout;

    closeButton = new DPushButton(this);
    closeButton->setText("<");
    closeButton->setFixedSize(QSize(36, 36));
    headerLayout->addWidget(closeButton);

    DLabel *title = new DLabel(this);
    title->setText(tr("History"));
    auto font = title->font();
    font.setPixelSize(16);
    title->setFont(font);
    headerLayout->addWidget(title);

    headerLayout->addStretch(1);

    mainLayout->addLayout(headerLayout);

    DWidget *itemsContainer = new DWidget(this);
    mainLayout->addWidget(itemsContainer);

    QVBoxLayout *containerLayout = new QVBoxLayout;
    containerLayout->setMargin(0);
    containerLayout->setSpacing(10);
    itemsContainer->setLayout(containerLayout);

    for (int i = 0; i < kMaxItemCountInOnePage; ++i) {
        auto item = new SessionRecordItem(itemsContainer);
        containerLayout->addWidget(item, 0);
        item->setVisible(false);

        itemsList.append(item);
        connect(item, &SessionRecordItem::closeHistoryWidget, this, &HistoryListWidget::requestCloseHistoryWidget);
    }

    mainLayout->addStretch(1);

    pageController = new PageControlComponent(this);
    mainLayout->addWidget(pageController);
    pageController->updatePageControl(10);
}

void HistoryListWidget::initConnection()
{
    connect(closeButton, &DPushButton::clicked, this, &HistoryListWidget::requestCloseHistoryWidget);
    connect(pageController, &PageControlComponent::pageChanged, this, &HistoryListWidget::onPageChanged);
    connect(ChatManager::instance(), &ChatManager::sessionRecordsUpdated, this, &HistoryListWidget::onSessionRecordUpdated);
}

void HistoryListWidget::updateItems(const QList<RecordData> &dataList)
{
    for (int i = 0; i < itemsList.count(); ++i) {
        if (i < dataList.length()) {
            auto record = dataList[i];
            itemsList[i]->updateItem(record);
            itemsList[i]->setVisible(true);
        } else {
            itemsList[i]->setVisible(false);
        }
    }
}

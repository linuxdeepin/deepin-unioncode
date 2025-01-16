// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagecontrolcomponent.h"

#include <DPushButton>

#include <QHBoxLayout>

PageControlComponent::PageControlComponent(QWidget *parent)
    : DWidget (parent),
      pageCount(1),
      currentPageIndex(0)
{
    initUI();
    initConnection();
}

void PageControlComponent::updatePageControl(int count)
{
    pageCount = count;

    if (pageCount > 1) {
        lastPageButton->setVisible(true);
        lastPageButton->setText(QString::number(count));
    } else {
        lastPageButton->setVisible(false);
    }

    if (currentPageIndex >= pageCount)
        currentPageIndex = pageCount - 1;

    updatePageButtons();
}

int PageControlComponent::currentIndex() const
{
    return currentPageIndex;
}

void PageControlComponent::onPreBtnClicked()
{
    if (currentPageIndex == 0)
        return;

    setCurrentPage(currentPageIndex - 1);
}

void PageControlComponent::onNextBtnClicked()
{
    if (currentPageIndex == pageCount - 1)
        return;

    setCurrentPage(currentPageIndex + 1);
}

void PageControlComponent::onLeftMoreBtnClicked()
{
    int newIndex = qMax(0, midPageButtonsList.at(0)->text().toInt() - 3);
    setCurrentPage(newIndex);
}

void PageControlComponent::onRightMoreBtnClicked()
{
    int newIndex = qMin(pageCount - 1, midPageButtonsList.at(2)->text().toInt() + 1);
    setCurrentPage(newIndex);
}

void PageControlComponent::onNumberBtnClicked()
{
    auto btn = qobject_cast<DPushButton*>(sender());
    if (!btn)
        return;

    setCurrentPage(btn->text().toInt() - 1);
}

void PageControlComponent::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(3);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    initPageButtons();
}

void PageControlComponent::initPageButtons()
{
    pageButtonsLayout = new QHBoxLayout;
    pageButtonsLayout->setSpacing(3);
    pageButtonsLayout->setContentsMargins(0, 0, 0, 0);

    preButton = createPushButton();
    preButton->setText("<");
    pageButtonsLayout->addWidget(preButton);

    firstPageButton = createPushButton();
    firstPageButton->setText("1");
    pageButtonsLayout->addWidget(firstPageButton);

    leftMoreButton = createPushButton();
    leftMoreButton->setText("...");
    pageButtonsLayout->addWidget(leftMoreButton);

    for (int i = 0; i < 3; ++i) {
        auto midBtn = createPushButton();
        pageButtonsLayout->addWidget(midBtn);
        midPageButtonsList.append(midBtn);
    }

    rightMoreButton = createPushButton();
    rightMoreButton->setText("...");
    pageButtonsLayout->addWidget(rightMoreButton);

    lastPageButton = createPushButton();
    pageButtonsLayout->addWidget(lastPageButton);

    nextButton = createPushButton();
    nextButton->setText(">");
    pageButtonsLayout->addWidget(nextButton);

    qobject_cast<QHBoxLayout*>(layout())->addLayout(pageButtonsLayout);
}

void PageControlComponent::initConnection()
{
    connect(preButton, &DPushButton::clicked, this, &PageControlComponent::onPreBtnClicked);
    connect(nextButton, &DPushButton::clicked, this, &PageControlComponent::onNextBtnClicked);
    connect(leftMoreButton, &DPushButton::clicked, this, &PageControlComponent::onLeftMoreBtnClicked);
    connect(rightMoreButton, &DPushButton::clicked, this, &PageControlComponent::onRightMoreBtnClicked);
    connect(firstPageButton, &DPushButton::clicked, this, &PageControlComponent::onNumberBtnClicked);
    connect(lastPageButton, &DPushButton::clicked, this, &PageControlComponent::onNumberBtnClicked);
    for (auto numberBtn : midPageButtonsList) {
        connect(numberBtn, &DPushButton::clicked, this, &PageControlComponent::onNumberBtnClicked);
    }
}

void PageControlComponent::updatePageButtons()
{
    bool showLeftMoreBtn { false };
    bool showRightMoreBtn { false };
    int midBeginPageIndex = 1;
    if (pageCount > 5) {
        if (currentPageIndex < pageCount - currentPageIndex + 1) {
            if (currentPageIndex > 2) {
                showLeftMoreBtn = true;
                midBeginPageIndex = currentPageIndex - 1;
            } else {
                midBeginPageIndex = 1;
            }
            showRightMoreBtn = true;
        } else {
            int leftSubIndex = 1;
            if (pageCount - currentPageIndex - 1 > 3) {
                showRightMoreBtn = true;
            } else {
                leftSubIndex += (3 - (pageCount - currentPageIndex));
            }
            showLeftMoreBtn = true;
            midBeginPageIndex = currentPageIndex - leftSubIndex;
        }
    }

    leftMoreButton->setVisible(showLeftMoreBtn);
    rightMoreButton->setVisible(showRightMoreBtn);

    for (int i = 0; i < midPageButtonsList.count(); ++i) {
        if (midBeginPageIndex + i < pageCount && pageCount > 2) {
            auto btn = midPageButtonsList[i];
            btn->setVisible(true);
            btn->setText(QString::number(indexToNumber(midBeginPageIndex + i)));
            if (midBeginPageIndex + i == currentPageIndex) {
                btn->setFlat(false);
            } else {
                btn->setFlat(true);
            }
        } else {
            midPageButtonsList[i]->setVisible(false);
        }
    }

    if (currentPageIndex == 0)
        firstPageButton->setFlat(false);
    else
        firstPageButton->setFlat(true);

    if (currentPageIndex == pageCount - 1)
        lastPageButton->setFlat(false);
    else
        lastPageButton->setFlat(true);
}

void PageControlComponent::setCurrentPage(int pageIndex)
{
    if (currentPageIndex == pageIndex
            || pageIndex < 0
            || pageIndex >= pageCount)
        return;

    currentPageIndex = pageIndex;
    updatePageButtons();

    Q_EMIT pageChanged(currentPageIndex);
}

DPushButton *PageControlComponent::createPushButton()
{
    DPushButton *btn = new DPushButton(this);
    btn->setFixedSize(32, 32);
    btn->installEventFilter(this);
    btn->setFlat(true);

    return btn;
}

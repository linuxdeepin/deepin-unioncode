// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexwidget.h"
#include "askpagewidget.h"
#include "translationpagewidget.h"
#include "codegeexmanager.h"

#include <QDebug>
#include <QTabBar>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

CodeGeeXWidget::CodeGeeXWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnection();
    CodeGeeXManager::instance()->queryLoginState();
}

void CodeGeeXWidget::onLoginSuccessed()
{
    if (loginBtn) {
        auto mainLayout = qobject_cast<QVBoxLayout*>(layout());
        mainLayout->removeWidget(loginBtn);
        delete loginBtn;
        loginBtn = nullptr;
    }

    initAskWidget();
    CodeGeeXManager::instance()->createNewSession();
}

void CodeGeeXWidget::onNewSessionCreated()
{
    stackWidget->setCurrentIndex(1);

    if (askPage)
        askPage->setIntroPage();
}

void CodeGeeXWidget::onDeleteBtnClicked()
{
    CodeGeeXManager::instance()->deleteCurrentSession();
    CodeGeeXManager::instance()->cleanHistoryMessage();
}

void CodeGeeXWidget::onHistoryBtnClicked()
{

}

void CodeGeeXWidget::onCreateNewBtnClicked()
{
    CodeGeeXManager::instance()->cleanHistoryMessage();
    CodeGeeXManager::instance()->createNewSession();
}

void CodeGeeXWidget::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    loginBtn = new QPushButton(this);
    loginBtn->setText(tr("Login"));
}

void CodeGeeXWidget::initConnection()
{
    connect(loginBtn, &QPushButton::clicked, CodeGeeXManager::instance(), &CodeGeeXManager::login);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::loginSuccessed, this, &CodeGeeXWidget::onLoginSuccessed);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::createdNewSession, this, &CodeGeeXWidget::onNewSessionCreated);
}

void CodeGeeXWidget::initAskWidget()
{
    tabBar = new QTabBar(this);
    tabBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tabBar->setContentsMargins(0, 0, 0, 0);
    stackWidget = new QStackedWidget(this);
    stackWidget->setContentsMargins(0, 0, 0, 0);
    stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto mainLayout = qobject_cast<QVBoxLayout*>(layout());
    mainLayout->setSpacing(0);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    mainLayout->addLayout(headerLayout, 0);
    mainLayout->addWidget(stackWidget, 1);

    headerLayout->addWidget(tabBar, 0);
    headerLayout->addStretch(1);

    deleteBtn = new QPushButton(this);
    deleteBtn->setText(tr("Delete"));
    headerLayout->addWidget(deleteBtn);
    historyBtn = new QPushButton(this);
    historyBtn->setText(tr("History"));
    headerLayout->addWidget(historyBtn);
    createNewBtn = new QPushButton(this);
    createNewBtn->setText(tr("New"));
    headerLayout->addWidget(createNewBtn);

    initTabBar();
    initStackWidget();
    initAskWidgetConnection();

    currentState = AskPage;
    resetHeaderBtns();
}

void CodeGeeXWidget::initTabBar()
{
    tabBar->setShape(QTabBar::TriangularNorth);
    tabBar->addTab(tr("Ask CodeGeeX"));
    tabBar->addTab(tr("Translation"));
}

void CodeGeeXWidget::initStackWidget()
{
    askPage = new AskPageWidget(this);
    transPage = new TranslationPageWidget(this);

    connect(askPage, &AskPageWidget::introPageShown, this, [ = ]{
        currentState = AskPage;
        resetHeaderBtns();
    });
    connect(askPage, &AskPageWidget::sessionPageShown, this, [ = ]{
        currentState = AskPage;
        resetHeaderBtns();
    });

    QWidget *creatingSessionWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    creatingSessionWidget->setLayout(layout);

    QLabel *creatingLabel = new QLabel(creatingSessionWidget);
    creatingLabel->setAlignment(Qt::AlignCenter);
    creatingLabel->setText(tr("Creating a new session..."));
    layout->addWidget(creatingLabel);

    stackWidget->insertWidget(0, creatingSessionWidget);
    stackWidget->insertWidget(1, askPage);
    stackWidget->insertWidget(2, transPage);
    stackWidget->setCurrentIndex(0);
}

void CodeGeeXWidget::initAskWidgetConnection()
{
    connect(tabBar, &QTabBar::currentChanged, this, [ = ](int index){
        if (index == 0) {
            currentState = AskPage;
            resetHeaderBtns();
        } else if (index == 1) {
            currentState = TrasnlatePage;
            resetHeaderBtns();
        }
        stackWidget->setCurrentIndex(index + 1);
    });
    connect(deleteBtn, &QPushButton::clicked, this, &CodeGeeXWidget::onDeleteBtnClicked);
    connect(historyBtn, &QPushButton::clicked, this, &CodeGeeXWidget::onHistoryBtnClicked);
    connect(createNewBtn, &QPushButton::clicked, this, &CodeGeeXWidget::onCreateNewBtnClicked);
}

void CodeGeeXWidget::resetHeaderBtns()
{
    if (!deleteBtn || !historyBtn || !createNewBtn || !askPage)
        return;

    switch (currentState) {
    case AskPage:
        deleteBtn->setVisible(!askPage->isIntroPageState());
        createNewBtn->setVisible(!askPage->isIntroPageState());
        historyBtn->setVisible(true);
        break;
    case TrasnlatePage:
        deleteBtn->setVisible(false);
        createNewBtn->setVisible(false);
        historyBtn->setVisible(false);
        break;
    }
}

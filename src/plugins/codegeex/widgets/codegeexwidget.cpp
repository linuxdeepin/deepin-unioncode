// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexwidget.h"
#include "askpagewidget.h"
#include "historylistwidget.h"
#include "translationpagewidget.h"
#include "codegeexmanager.h"
#include "copilot.h"

#include <DLabel>
#include <DStackedWidget>

#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QResizeEvent>

CodeGeeXWidget::CodeGeeXWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnection();
}

void CodeGeeXWidget::onLoginSuccessed()
{
    auto mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        QLayoutItem* item = nullptr;
        while ((item = mainLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    initAskWidget();
    initHistoryWidget();
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
    CodeGeeXManager::instance()->fetchSessionRecords();

    if (!historyWidget || !historyWidgetAnimation)
        return;

    historyWidgetAnimation->setStartValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = true;
}

void CodeGeeXWidget::onCreateNewBtnClicked()
{
    CodeGeeXManager::instance()->cleanHistoryMessage();
    CodeGeeXManager::instance()->createNewSession();
}

void CodeGeeXWidget::toTranslateCode(const QString &code)
{
    currentState = TrasnlatePage;
    resetHeaderBtns();

    transPage->setInputEditText(code);
    transPage->cleanOutputEdit();

    tabBar->setCurrentIndex(1);
}

void CodeGeeXWidget::onAnwserFinished()
{
    if (deleteBtn)
        deleteBtn->setEnabled(true);
    if (historyBtn)
        historyBtn->setEnabled(true);
    if (createNewBtn)
        createNewBtn->setEnabled(true);
}

void CodeGeeXWidget::onAnwserStarted()
{
    if (deleteBtn)
        deleteBtn->setEnabled(false);
    if (historyBtn)
        historyBtn->setEnabled(false);
    if (createNewBtn)
        createNewBtn->setEnabled(false);
}

void CodeGeeXWidget::onCloseHistoryWidget()
{
    historyWidgetAnimation->setStartValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = false;
}

void CodeGeeXWidget::resizeEvent(QResizeEvent *event)
{
    if (historyWidget) {
        if (historyShowed) {
            historyWidget->setGeometry(0, 0, this->width(), this->height());
        } else {
            historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
        }
    }

    DWidget::resizeEvent(event);
}

void CodeGeeXWidget::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto initLoginUI = [this](){
        auto verticalLayout = new QVBoxLayout(this);
        verticalLayout->setAlignment(Qt::AlignCenter);
        auto verticalSpacer_top = new QSpacerItem(20, 200, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout->addItem(verticalSpacer_top);

        auto label_icon = new DLabel();
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        label_icon->setSizePolicy(sizePolicy);
        label_icon->setPixmap(QIcon::fromTheme("codegeex_anwser_icon").pixmap(QSize(80, 80)));
        label_icon->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_icon, Qt::AlignCenter);

        auto label_text = new DLabel();
        label_text->setSizePolicy(sizePolicy);
        label_text->setText(tr("Welcome to CodeGeeX\nA must-have all-round AI tool for developers"));
        label_text->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_text, Qt::AlignCenter);

        auto loginBtn = new DPushButton();
        loginBtn->setSizePolicy(sizePolicy);
        loginBtn->setText(tr("Go to login"));
        connect(loginBtn, &DPushButton::clicked, this, [ = ]{
            qInfo() << "on login clicked";
            CodeGeeXManager::instance()->login();
        });

        verticalLayout->addWidget(loginBtn);

        auto verticalSpacer_bottom = new QSpacerItem(20, 500, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout->addItem(verticalSpacer_bottom);
    };
    initLoginUI();
}

void CodeGeeXWidget::initConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::loginSuccessed, this, &CodeGeeXWidget::onLoginSuccessed);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::createdNewSession, this, &CodeGeeXWidget::onNewSessionCreated);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestToTransCode, this, &CodeGeeXWidget::toTranslateCode);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatFinished, this, &CodeGeeXWidget::onAnwserFinished);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatStarted, this, &CodeGeeXWidget::onAnwserStarted);
}

void CodeGeeXWidget::initAskWidget()
{
    tabBar = new DTabBar(this);
    tabBar->setVisibleAddButton(false);
    tabBar->setUsesScrollButtons(false);
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

    deleteBtn = new DPushButton(this);
    deleteBtn->setFlat(true);
    deleteBtn->setIcon(QIcon::fromTheme("codegeex_clear"));
    headerLayout->addWidget(deleteBtn);
    historyBtn = new DPushButton(this);
    historyBtn->setFlat(true);
    historyBtn->setIcon(QIcon::fromTheme("codegeex_history"));
    headerLayout->addWidget(historyBtn);
    createNewBtn = new DPushButton(this);
    createNewBtn->setFlat(true);
    createNewBtn->setIcon(QIcon::fromTheme("codegeex_new"));
    headerLayout->addWidget(createNewBtn);

    initTabBar();
    initStackWidget();
    initAskWidgetConnection();

    currentState = AskPage;
    resetHeaderBtns();
}

void CodeGeeXWidget::initHistoryWidget()
{
    historyWidget = new HistoryListWidget(this);
    historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
    historyWidget->show();

    historyWidgetAnimation =  new QPropertyAnimation(historyWidget, "geometry");
    historyWidgetAnimation->setEasingCurve(QEasingCurve::InOutSine);
    historyWidgetAnimation->setDuration(300);

    initHistoryWidgetConnection();
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

    DWidget *creatingSessionWidget = new DWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    creatingSessionWidget->setLayout(layout);

    DLabel *creatingLabel = new DLabel(creatingSessionWidget);
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
    connect(tabBar, &DTabBar::currentChanged, this, [ = ](int index){
        if (index == 0) {
            currentState = AskPage;
            resetHeaderBtns();
        } else if (index == 1) {
            currentState = TrasnlatePage;
            resetHeaderBtns();
        }
        stackWidget->setCurrentIndex(index + 1);
    });
    connect(deleteBtn, &DPushButton::clicked, this, &CodeGeeXWidget::onDeleteBtnClicked);
    connect(historyBtn, &DPushButton::clicked, this, &CodeGeeXWidget::onHistoryBtnClicked);
    connect(createNewBtn, &DPushButton::clicked, this, &CodeGeeXWidget::onCreateNewBtnClicked);
}

void CodeGeeXWidget::initHistoryWidgetConnection()
{
    connect(historyWidget, &HistoryListWidget::requestCloseHistoryWidget, this, &CodeGeeXWidget::onCloseHistoryWidget);
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

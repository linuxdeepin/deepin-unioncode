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

CodeGeeXWidget::CodeGeeXWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initLoginConnection();
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
}

void CodeGeeXWidget::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    loginBtn = new QPushButton(this);
    loginBtn->setText(tr("Login"));

    connect(loginBtn, &QPushButton::clicked, this, [ = ]{
        qInfo() << "on login clicked";
        CodeGeeXManager::instance()->login();
    });
}

void CodeGeeXWidget::initLoginConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::loginSuccessed, this, &CodeGeeXWidget::onLoginSuccessed);
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
    mainLayout->addWidget(tabBar, 0);
    mainLayout->addWidget(stackWidget, 1);

    initTabBar();
    initStackWidget();
    initAskWidgetConnection();
}

void CodeGeeXWidget::initTabBar()
{
    tabBar->setShape(QTabBar::TriangularNorth);
    tabBar->addTab("Ask CodeGeeX");
    tabBar->addTab("Translation");
}

void CodeGeeXWidget::initStackWidget()
{
    AskPageWidget *askPage = new AskPageWidget;
    TranslationPageWidget *transPage = new TranslationPageWidget;

    stackWidget->insertWidget(0, askPage);
    stackWidget->insertWidget(1, transPage);
    stackWidget->setCurrentIndex(0);

    askPage->setIntroPage();
}

void CodeGeeXWidget::initAskWidgetConnection()
{
    connect(tabBar, &QTabBar::currentChanged, stackWidget, &QStackedWidget::setCurrentIndex);
}

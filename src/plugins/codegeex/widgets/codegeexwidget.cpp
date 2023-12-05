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
    : DFrame(parent)
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

void CodeGeeXWidget::toTranslateCode(const QString &code)
{
    transPage->setInputEditText(code);
    transPage->cleanOutputEdit();
}

void CodeGeeXWidget::onCloseHistoryWidget()
{
    historyWidgetAnimation->setStartValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = false;
}

void CodeGeeXWidget::onShowHistoryWidget()
{
    CodeGeeXManager::instance()->fetchSessionRecords();

    if (!historyWidget || !historyWidgetAnimation)
        return;

    historyWidgetAnimation->setStartValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = true;
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
    //todo(zta) : 低于330宽度时显示有异常，先限制最小宽度，后续找原因
    setMinimumWidth(330);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto initLoginUI = [this](){
        auto verticalLayout = new QVBoxLayout(this);
        verticalLayout->setMargin(0);
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
}

void CodeGeeXWidget::initAskWidget()
{
    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->setContentsMargins(0, 20, 0, 0);
    tabLayout->setAlignment(Qt::AlignHCenter);

    tabBar = new DButtonBox(this);
    tabLayout->addWidget(tabBar, 0);

    stackWidget = new QStackedWidget(this);
    stackWidget->setContentsMargins(0, 0, 0, 0);
    stackWidget->setFrameShape(QFrame::NoFrame);
    stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto mainLayout = qobject_cast<QVBoxLayout*>(layout());
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addLayout(tabLayout, 0);
    mainLayout->addWidget(stackWidget, 1);

    initTabBar();
    initStackWidget();
    initAskWidgetConnection();
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
    DButtonBoxButton *askbtn = new DButtonBoxButton(tr("Ask CodeGeeX"), this);
    askbtn->setCheckable(true);
    askbtn->setChecked(true);
    DButtonBoxButton *trsbtn = new DButtonBoxButton(tr("Translation"), this);

    tabBar->setButtonList({askbtn, trsbtn}, true);
}

void CodeGeeXWidget::initStackWidget()
{
    askPage = new AskPageWidget(this);
    transPage = new TranslationPageWidget(this);

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
    connect(tabBar, &DButtonBox::buttonClicked, stackWidget, [ = ] (QAbstractButton *button){
        if(button->text() == tr("Ask CodeGeeX"))
            stackWidget->setCurrentWidget(askPage);
        else
            stackWidget->setCurrentWidget(transPage);
    });
    connect(askPage, &AskPageWidget::requestShowHistoryPage, this, &CodeGeeXWidget::onShowHistoryWidget);
}

void CodeGeeXWidget::initHistoryWidgetConnection()
{
    connect(historyWidget, &HistoryListWidget::requestCloseHistoryWidget, this, &CodeGeeXWidget::onCloseHistoryWidget);
}

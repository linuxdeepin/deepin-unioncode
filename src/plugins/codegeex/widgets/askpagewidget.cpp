// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "askpagewidget.h"
#include "intropage.h"
#include "messagecomponent.h"
#include "codegeexmanager.h"
#include "services/editor/editorservice.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DScrollArea>
#include <DScrollBar>
#include <DHorizontalLine>
#include <DDialog>

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QKeyEvent>
#include <QMenu>
#include <QFileDialog>

static const int minInputEditHeight = 36;
static const int maxInputEditHeight = 236;
static const int minInputWidgetHeight = 86;

InputEdit::InputEdit(QWidget *parent)
    : DTextEdit(parent)
{
    setMinimumHeight(minInputEditHeight);
    setFixedHeight(minInputEditHeight);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, &DTextEdit::textChanged, this, [this]() {
        auto adjustHeight = document()->size().height();
        if (adjustHeight < minInputEditHeight)
            setFixedHeight(minInputEditHeight);
        else if (adjustHeight > maxInputEditHeight)
            setFixedHeight(maxInputEditHeight);
        else
            setFixedHeight(adjustHeight);
    });
}

void InputEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        if (e->modifiers() & Qt::AltModifier)
            insertPlainText("\n");
        else if (!document()->toPlainText().isEmpty())
            emit pressedEnter();
        return;
    }

    DTextEdit::keyPressEvent(e);
}

AskPageWidget::AskPageWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnection();
}

void AskPageWidget::setIntroPage()
{
    cleanWidgets();
    curState = Intro;
    IntroPage *introPage = new IntroPage(scrollArea);
    connect(introPage, &IntroPage::suggestionToSend, [this](const QString &suggesstion) {
        askQuestion(suggesstion);
    });
    scrollArea->setWidget(introPage);

    resetBtns();

    Q_EMIT introPageShown();
}

bool AskPageWidget::isIntroPageState()
{
    return curState == Intro;
}

void AskPageWidget::onMessageUpdate(const MessageData &msgData)
{
    if (curState == Intro) {
        curState = Session;
        setSessionPage();
    }

    if (!msgComponents.contains(msgData.messageID())) {
        if (waitingAnswer && msgData.messageType() == MessageData::Anwser) {
            msgComponents.insert(msgData.messageID(), waitComponets);
            waitingAnswer = false;
        } else {
            msgComponents.insert(msgData.messageID(), new MessageComponent(msgData, messageContainer));
            qobject_cast<QVBoxLayout *>(messageContainer->layout())->insertWidget(msgComponents.count() - 1, msgComponents.value(msgData.messageID()));
        }

        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    } else {
        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    }
}

void AskPageWidget::slotMessageSend()
{
    if (inputEdit) {
        auto prompt = inputEdit->toPlainText();
        if (prompt.isEmpty())
            return;
        askQuestion(prompt);
    }
}

void AskPageWidget::onChatFinished()
{
    enterInputState();
}

void AskPageWidget::onDeleteBtnClicked()
{
    DDialog *confirmDialog = new DDialog(this);
    confirmDialog->setIcon(QIcon::fromTheme("dialog-warning"));
    confirmDialog->setMessage(tr("This operation will delete all the content of this session. confirm to delete it?"));
    confirmDialog->insertButton(0, tr("Cancel", "button"));
    confirmDialog->insertButton(1, tr("Delete", "button"), false, DDialog::ButtonWarning);

    connect(confirmDialog, &DDialog::buttonClicked, this, [](int index) {
        if (index == 1) {
            CodeGeeXManager::instance()->deleteCurrentSession();
            CodeGeeXManager::instance()->cleanHistoryMessage();
        }
    });

    confirmDialog->exec();
}

void AskPageWidget::onReferenceBtnClicked()
{
    referenceBtn->setChecked(true);
    referenceMenu->exec(QCursor::pos());
    referenceBtn->update();
    if (selectedFiles.isEmpty())
        referenceBtn->setChecked(false);

    CodeGeeXManager::instance()->setRefereceFiles(selectedFiles);
}

void AskPageWidget::onNetWorkBtnClicked()
{
    CodeGeeXManager::instance()->connectToNetWork(netWorkBtn->isChecked());
}

void AskPageWidget::onHistoryBtnClicked()
{
    Q_EMIT requestShowHistoryPage();
}

void AskPageWidget::onCreateNewBtnClicked()
{
    CodeGeeXManager::instance()->cleanHistoryMessage();
    CodeGeeXManager::instance()->createNewSession();
}

void AskPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    scrollArea = new DScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter);
    layout->addWidget(scrollArea);

    //套一层Widget用以隐藏button和layout
    stopWidget = new DWidget(this);
    QHBoxLayout *hlayout = new QHBoxLayout;
    stopWidget->setLayout(hlayout);
    stopGenerate = new DPushButton(this);
    stopGenerate->setText(tr("stop generate"));
    hlayout->setContentsMargins(0, 20, 0, 20);
    hlayout->addWidget(stopGenerate);
    hlayout->setAlignment(Qt::AlignHCenter);
    layout->addWidget(stopWidget);
    stopWidget->hide();

    DHorizontalLine *line = new DHorizontalLine(this);
    layout->addWidget(line);

    inputWidget = new DWidget(this);
    layout->addWidget(inputWidget);

    initInputWidget();
}

void AskPageWidget::initInputWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);
    inputWidget->setLayout(layout);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    layout->addLayout(btnLayout);
    btnLayout->setContentsMargins(0, 0, 0, 0);

    deleteBtn = new DToolButton(this);
    deleteBtn->setFixedSize(30, 30);
    deleteBtn->setIcon(QIcon::fromTheme("codegeex_clear"));
    deleteBtn->setToolTip(tr("delete this session"));

    referenceBtn = new DToolButton(this);
    referenceBtn->setFixedSize(30, 30);
    referenceBtn->setIcon(QIcon::fromTheme("codegeex_files"));
    referenceBtn->setToolTip(tr("reference files"));
    referenceBtn->setCheckable(true);

    netWorkBtn = new DToolButton(this);
    netWorkBtn->setFixedSize(30, 30);
    netWorkBtn->setCheckable(true);
    netWorkBtn->setIcon(QIcon::fromTheme("codegeex_internet"));
    netWorkBtn->setToolTip(tr("connect to network"));

    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(referenceBtn);
    btnLayout->addWidget(netWorkBtn);

    btnLayout->addStretch(1);

    historyBtn = new DToolButton(this);
    historyBtn->setIcon(QIcon::fromTheme("codegeex_history"));
    historyBtn->setFixedSize(30, 30);
    historyBtn->setToolTip(tr("history sessions"));
    btnLayout->addWidget(historyBtn);

    createNewBtn = new DToolButton(this);
    createNewBtn->setIcon(QIcon::fromTheme("codegeex_new"));
    createNewBtn->setFixedSize(30, 30);
    createNewBtn->setToolTip(tr("create new session"));
    btnLayout->addWidget(createNewBtn);

    auto hlayout = new QHBoxLayout;
    inputEdit = new InputEdit(inputWidget);
    placeHolderText = tr("Ask question here, press Enter to send...");
    inputEdit->setPlaceholderText(placeHolderText);

    sendButton = new DFloatingButton(this);
    sendButton->setFixedSize(30, 30);
    sendButton->setIcon(QIcon::fromTheme("codegeex_send").pixmap(16, QIcon::Selected));
    sendButton->setEnabled(false);

    inputWidget->setFixedHeight(minInputWidgetHeight);
    inputWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    hlayout->addWidget(inputEdit);
    hlayout->setSpacing(10);
    hlayout->addWidget(sendButton);
    layout->addLayout(hlayout);

    initReferenceMenu();
}

void AskPageWidget::initConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestMessageUpdate, this, &AskPageWidget::onMessageUpdate);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatStarted, this, &AskPageWidget::enterAnswerState);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::chatFinished, this, &AskPageWidget::onChatFinished);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::terminated, this, &AskPageWidget::onChatFinished);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::setTextToSend, this, &AskPageWidget::setInputText);

    connect(sendButton, &DFloatingButton::clicked, this, &AskPageWidget::slotMessageSend);
    connect(inputEdit, &InputEdit::pressedEnter, this, &AskPageWidget::slotMessageSend);
    connect(deleteBtn, &DToolButton::clicked, this, &AskPageWidget::onDeleteBtnClicked);
    connect(referenceBtn, &DToolButton::clicked, this, &AskPageWidget::onReferenceBtnClicked);
    connect(netWorkBtn, &DToolButton::clicked, this, &AskPageWidget::onNetWorkBtnClicked);
    connect(historyBtn, &DToolButton::clicked, this, &AskPageWidget::onHistoryBtnClicked);
    connect(createNewBtn, &DToolButton::clicked, this, &AskPageWidget::onCreateNewBtnClicked);
    connect(inputEdit, &DTextEdit::textChanged, this, [this]() {
        if (inputEdit->toPlainText().isEmpty())
            sendButton->setEnabled(false);
        else
            sendButton->setEnabled(true);

        inputWidget->setFixedHeight(inputEdit->height() + minInputWidgetHeight - minInputEditHeight);
    });
    connect(stopGenerate, &DPushButton::clicked, this, [this]() {
        CodeGeeXManager::instance()->stopReceiving();
        Q_EMIT CodeGeeXManager::instance()->terminated();
        if (!msgComponents.values().contains(waitComponets)) {
            QString stopId = "Stop:" + QString::number(QDateTime::currentMSecsSinceEpoch());
            msgComponents.insert(stopId, waitComponets);
        }
        waitComponets->stopWaiting();
        waitingAnswer = false;
    });
    connect(scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged, this, [=]() {
        if (scrollArea->verticalScrollBar()->isVisible()) {
            int maxValue = scrollArea->verticalScrollBar()->maximum();
            scrollArea->verticalScrollBar()->setValue(maxValue);
        }
    });
}

void AskPageWidget::initReferenceMenu()
{
    // files references menu
    referenceMenu = new QMenu(this);
    auto *current = new QAction(tr("Current file"), this);
    current->setCheckable(true);
    auto *opened = new QAction(tr("Opened files"), this);
    opened->setCheckable(true);
    auto *select = new QAction(tr("Select file"), this);
    select->setCheckable(true);

    auto *clear = new QAction(tr("clear"), this);

    QActionGroup *group = new QActionGroup(this);
    group->addAction(current);
    group->addAction(opened);
    group->addAction(select);

    referenceMenu->addActions(group->actions());
    referenceMenu->addSeparator();
    referenceMenu->addAction(clear);

    auto editorSrv = dpfGetService(dpfservice::EditorService);

    connect(current, &QAction::triggered, this, [=](){
        selectedFiles.clear();
        auto file = editorSrv->currentFile();
        if (!file.isEmpty())
            selectedFiles.append(file);
    });
    connect(opened, &QAction::triggered, this, [=](){
        selectedFiles.clear();
        selectedFiles = editorSrv->openedFiles();
    });
    connect(select, &QAction::triggered, this, [=](){
        selectedFiles.clear();
        QString result = QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath());
        selectedFiles.append(result);
    });
    connect(clear, &QAction::triggered, this, [=](){
        selectedFiles.clear();
        referenceBtn->setChecked(false);
        group->checkedAction()->setChecked(false);
    });
}

void AskPageWidget::cleanWidgets()
{
    if (auto currentWidget = scrollArea->takeWidget()) {
        currentWidget->deleteLater();
        msgComponents.clear();
    }
}

void AskPageWidget::setSessionPage()
{
    cleanWidgets();
    messageContainer = new DWidget(scrollArea);
    scrollArea->setWidget(messageContainer);

    QVBoxLayout *layout = new QVBoxLayout(messageContainer);
    layout->setContentsMargins(12, 0, 12, 20);
    layout->setSpacing(10);
    messageContainer->setLayout(layout);

    layout->addStretch(1);

    resetBtns();

    Q_EMIT sessionPageShown();
}

void AskPageWidget::enterAnswerState()
{
    if (curState == Intro) {
        curState = Session;
        setSessionPage();
    }

    progressCalcNum = 0;
    inputEdit->clear();
    inputEdit->setEnabled(false);

    if (deleteBtn)
        deleteBtn->setEnabled(false);
    if (historyBtn)
        historyBtn->setEnabled(false);
    if (createNewBtn)
        createNewBtn->setEnabled(false);

    stopWidget->show();
    waitForAnswer();
}

void AskPageWidget::enterInputState()
{
    stopWidget->hide();
    inputEdit->setEnabled(true);
    inputEdit->setPlaceholderText(placeHolderText);

    if (deleteBtn)
        deleteBtn->setEnabled(true);
    if (historyBtn)
        historyBtn->setEnabled(true);
    if (createNewBtn)
        createNewBtn->setEnabled(true);
}

void AskPageWidget::waitForAnswer()
{
    waitingAnswer = true;
    MessageData data("", MessageData::Anwser);
    waitComponets = new MessageComponent(data, messageContainer);
    qobject_cast<QVBoxLayout *>(messageContainer->layout())->insertWidget(msgComponents.count(), waitComponets);
    waitComponets->waitForAnswer();
}

void AskPageWidget::askQuestion(const QString &question)
{
    CodeGeeXManager::instance()->sendMessage(question);
}

void AskPageWidget::resetBtns()
{
    if (!deleteBtn || !historyBtn || !createNewBtn)
        return;

    deleteBtn->setEnabled(!isIntroPageState());
    createNewBtn->setVisible(!isIntroPageState());
    historyBtn->setVisible(true);
}

void AskPageWidget::setInputText(const QString &prompt)
{
    if (!waitingAnswer)
        inputEdit->setText(prompt);
}

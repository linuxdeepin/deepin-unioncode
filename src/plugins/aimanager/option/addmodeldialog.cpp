// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "addmodeldialog.h"
#include "services/ai/aiservice.h"
#include "aimanager.h"

#include <DPasswordEdit>
#include <DStackedWidget>
#include <DSuggestButton>
#include <DLineEdit>
#include <DComboBox>
#include <DStyle>
#include <DVerticalLine>
#include <DLabel>
#include <DSpinner>

DWIDGET_USE_NAMESPACE

class AddModelDialogPrivate
{
public:
    friend class AddModelDialog;
    AddModelDialogPrivate(AddModelDialog *qq) : q(qq) {}
    void initUi();
    void initConnection();

private slots:
    void slotAddModel();

private:
    DStackedWidget *stackWidget { nullptr };
    DWidget *mainWidget { nullptr };
    DWidget *checkingWidget { nullptr };

    DLineEdit *leLLMName { nullptr };
    DComboBox *cbLLMType { nullptr };
    DLineEdit *leApiUrl { nullptr };
    DPasswordEdit *leApiKey { nullptr };
    
    DSuggestButton *okButton { nullptr };
    DPushButton *cancelButton { nullptr };
    DSpinner *spinner { nullptr };

    void showWaitingState(bool waiting);
    void showErrorInfoDialog(const QString &error);
    AddModelDialog *q;
    LLMInfo LLMAppended;
};

void AddModelDialogPrivate::initUi()
{
    q->setFixedWidth(543);
    QLabel *lbModelName = new QLabel(AddModelDialog::tr("Model Name"));
    lbModelName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leLLMName = new DLineEdit(q);
    leLLMName->setPlaceholderText(AddModelDialog::tr("Required, please enter."));

    QLabel *lbLLMType = new QLabel(AddModelDialog::tr("Model Type"));
    lbLLMType->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbLLMType = new DComboBox(q);
    cbLLMType->addItem(AddModelDialog::tr("OpenAi(Compatible)"), LLMType::OPENAI);

    QLabel *lbApiUrl = new QLabel(AddModelDialog::tr("Api Path"));
    lbApiUrl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leApiUrl = new DLineEdit(q);
    leApiUrl->setPlaceholderText(AddModelDialog::tr("Required, please enter."));

    QLabel *lbApiKey = new QLabel(AddModelDialog::tr("Api Key"));
    lbApiKey->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leApiKey = new DPasswordEdit(q);
    leApiKey->setPlaceholderText(AddModelDialog::tr("Optional, please enter."));

    q->setWindowTitle(AddModelDialog::tr("Add Model"));

    mainWidget = new QWidget(q);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    QGridLayout *gridLayout = new QGridLayout;

    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(10);

    gridLayout->addWidget(lbModelName, 0, 0);
    gridLayout->addWidget(leLLMName, 0, 1);

    gridLayout->addWidget(lbLLMType, 1, 0);
    gridLayout->addWidget(cbLLMType, 1, 1);

    gridLayout->addWidget(lbApiUrl, 2, 0);
    gridLayout->addWidget(leApiUrl, 2, 1);

    gridLayout->addWidget(lbApiKey, 3, 0);
    gridLayout->addWidget(leApiKey, 3, 1);

    okButton = new DSuggestButton(q);
    okButton->setText(AddModelDialog::tr("Confirm"));
    cancelButton = new DPushButton(q);
    cancelButton->setText(AddModelDialog::tr("Cancel"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(new DVerticalLine(q));
    buttonLayout->addWidget(okButton);

    mainLayout->addLayout(gridLayout);
    mainLayout->addSpacing(10);
    auto label = new DLabel(AddModelDialog::tr("To test if the model is available, the system will send a small amount of information, which will consume a small amount of tokens."), q);
    label->setWordWrap(true);
    label->setForegroundRole(DPalette::PlaceholderText);
    mainLayout->addWidget(label);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    // check llm valid
    checkingWidget = new QWidget;
    auto checkingLayout = new QVBoxLayout(checkingWidget);
    checkingLayout->setSpacing(20);
    spinner = new DSpinner(checkingWidget);
    spinner->setFixedSize(32, 32);
    QLabel *lbCheck = new QLabel(AddModelDialog::tr("Checking... please wait."), checkingWidget);
    lbCheck->setAlignment(Qt::AlignCenter);
    checkingLayout->addStretch(1);
    checkingLayout->addWidget(spinner, 0, Qt::AlignCenter);
    checkingLayout->addWidget(lbCheck, 0, Qt::AlignCenter);
    checkingLayout->addStretch(1);

    stackWidget = new DStackedWidget(q);
    stackWidget->addWidget(mainWidget);
    stackWidget->addWidget(checkingWidget);
    q->addContent(stackWidget);
}

void AddModelDialogPrivate::initConnection()
{
    AddModelDialog::connect(okButton, &DSuggestButton::clicked, q, [=](){
        slotAddModel();
    });
    AddModelDialog::connect(cancelButton, &DSuggestButton::clicked, q, &AddModelDialog::reject);
}

void AddModelDialogPrivate::slotAddModel()
{
    LLMInfo newLLMInfo;
    newLLMInfo.modelName = leLLMName->text();
    if (newLLMInfo.modelName.isEmpty()) {
        leLLMName->showAlertMessage(AddModelDialog::tr("This field cannot be empty."));
        return;
    }
    newLLMInfo.type = cbLLMType->currentData().value<LLMType>();
    newLLMInfo.modelPath = leApiUrl->text();
    if (newLLMInfo.modelPath.isEmpty()) {
        leApiUrl->showAlertMessage(AddModelDialog::tr("This field cannot be empty."));
        return;
    }
    newLLMInfo.apikey = leApiKey->text();

    showWaitingState(true);
    QString errStr;
    auto valid = AiManager::instance()->checkModelValid(newLLMInfo, &errStr);
    if (valid) {
        LLMAppended = newLLMInfo;
        q->accept();
    } else if (!errStr.isEmpty() && q->isVisible()) { // if dialog closed, do not show error info
        showErrorInfoDialog(errStr);
    }
    showWaitingState(false);
}

void AddModelDialogPrivate::showWaitingState(bool waiting)
{
    if (waiting) {
        spinner->start();
        stackWidget->setCurrentWidget(checkingWidget);
    } else {
        spinner->stop();
        stackWidget->setCurrentWidget(mainWidget);
    }
}

void AddModelDialogPrivate::showErrorInfoDialog(const QString &error)
{
    DDialog dialog;
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.setWindowTitle(AddModelDialog::tr("Error Information"));
    auto warningText = new DLabel(error, q);
    warningText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    warningText->setWordWrap(true);

    DFrame *mainWidget = new DFrame(&dialog);
    mainWidget->setFixedSize(360, 140);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(warningText, 0, Qt::AlignTop | Qt::AlignLeft);
    dialog.addContent(mainWidget);

    dialog.addButton(AddModelDialog::tr("Confirm"));
    dialog.exec(); 
}

AddModelDialog::AddModelDialog(QWidget *parent)
: DDialog(parent), d(new AddModelDialogPrivate(this))
{
    d->initUi();
    d->initConnection();
}

AddModelDialog::~AddModelDialog()
{
    delete d;
}

LLMInfo AddModelDialog::getNewLLmInfo()
{
    if (!d->LLMAppended.modelName.isEmpty())
        return d->LLMAppended;
    else
        return LLMInfo();
}

// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modelconfigdialog.h"
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
    friend class ModelConfigDialog;
    AddModelDialogPrivate(ModelConfigDialog *qq) : q(qq) {}
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
    ModelConfigDialog *q;
    LLMInfo LLMAppended;
};

void AddModelDialogPrivate::initUi()
{
    q->setFixedWidth(543);
    QLabel *lbModelName = new QLabel(ModelConfigDialog::tr("Model Name"));
    lbModelName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leLLMName = new DLineEdit(q);
    leLLMName->setPlaceholderText(ModelConfigDialog::tr("Required, please enter."));

    QLabel *lbLLMType = new QLabel(ModelConfigDialog::tr("Model Type"));
    lbLLMType->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbLLMType = new DComboBox(q);
    cbLLMType->addItem(ModelConfigDialog::tr("OpenAi(Compatible)"), LLMType::OPENAI);

    QLabel *lbApiUrl = new QLabel(ModelConfigDialog::tr("Api Path"));
    lbApiUrl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leApiUrl = new DLineEdit(q);
    leApiUrl->setPlaceholderText(ModelConfigDialog::tr("Root path, such as https://server/v1 (no /chat)"));

    QLabel *lbApiKey = new QLabel(ModelConfigDialog::tr("Api Key"));
    lbApiKey->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leApiKey = new DPasswordEdit(q);
    leApiKey->setPlaceholderText(ModelConfigDialog::tr("Optional, please enter."));

    q->setWindowTitle(ModelConfigDialog::tr("Add Model"));

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
    okButton->setText(ModelConfigDialog::tr("Confirm"));
    cancelButton = new DPushButton(q);
    cancelButton->setText(ModelConfigDialog::tr("Cancel"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(new DVerticalLine(q));
    buttonLayout->addWidget(okButton);

    mainLayout->addLayout(gridLayout);
    mainLayout->addSpacing(10);
    auto label = new DLabel(ModelConfigDialog::tr("To test if the model is available, the system will send a small amount of information, which will consume a small amount of tokens."), q);
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
    QLabel *lbCheck = new QLabel(ModelConfigDialog::tr("Checking... please wait."), checkingWidget);
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
    ModelConfigDialog::connect(okButton, &DSuggestButton::clicked, q, [=](){
        slotAddModel();
    });
    ModelConfigDialog::connect(cancelButton, &DSuggestButton::clicked, q, &ModelConfigDialog::reject);
}

void AddModelDialogPrivate::slotAddModel()
{
    LLMInfo newLLMInfo;
    newLLMInfo.modelName = leLLMName->text();
    if (newLLMInfo.modelName.isEmpty()) {
        leLLMName->showAlertMessage(ModelConfigDialog::tr("This field cannot be empty."));
        return;
    }
    newLLMInfo.type = cbLLMType->currentData().value<LLMType>();
    if (newLLMInfo.type == LLMType::OPENAI)
        newLLMInfo.icon = QIcon::fromTheme("uc_openai");

    newLLMInfo.modelPath = leApiUrl->text();
    if (newLLMInfo.modelPath.isEmpty()) {
        leApiUrl->showAlertMessage(ModelConfigDialog::tr("This field cannot be empty."));
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
    dialog.setWindowTitle(ModelConfigDialog::tr("Error Information"));
    auto warningText = new DLabel(error, q);
    warningText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    warningText->setWordWrap(true);

    DFrame *mainWidget = new DFrame(&dialog);
    mainWidget->setFixedSize(360, 140);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(warningText, 0, Qt::AlignTop | Qt::AlignLeft);
    dialog.addContent(mainWidget);

    dialog.addButton(ModelConfigDialog::tr("Confirm"));
    dialog.exec(); 
}

ModelConfigDialog::ModelConfigDialog(QWidget *parent)
: DDialog(parent), d(new AddModelDialogPrivate(this))
{
    d->initUi();
    d->initConnection();
}

ModelConfigDialog::~ModelConfigDialog()
{
    delete d;
}

LLMInfo ModelConfigDialog::getLLmInfo()
{
    if (!d->LLMAppended.modelName.isEmpty())
        return d->LLMAppended;
    else
        return LLMInfo();
}

void ModelConfigDialog::setLLmInfo(const LLMInfo &llmInfo)
{
    d->leLLMName->setText(llmInfo.modelName);

    int index = d->cbLLMType->findData(QVariant::fromValue(llmInfo.type));
    if (index != -1) {
        d->cbLLMType->setCurrentIndex(index);
    }

    d->leApiUrl->setText(llmInfo.modelPath);
    d->leApiKey->setText(llmInfo.apikey);
}

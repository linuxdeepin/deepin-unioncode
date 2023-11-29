// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "translationpagewidget.h"
#include "codeeditcomponent.h"
#include "copilot.h"
#include "codegeex/copilotapi.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DComboBox>
#include <DPushButton>
#include <DLabel>
#include <QDebug>

TranslationPageWidget::TranslationPageWidget(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnection();
}

void TranslationPageWidget::setInputEditText(const QString &text)
{
    if (inputEdit)
        inputEdit->updateCode(text);
}

void TranslationPageWidget::cleanOutputEdit()
{
    if (outputEdit)
        outputEdit->updateCode("");
}

void TranslationPageWidget::onTranslateBtnClicked()
{
    QString dstLang = langComboBox->currentText();
    QString srcCode = inputEdit->getContent();
    Copilot::instance()->translateCode(srcCode, dstLang);
}

void TranslationPageWidget::onRecevieTransCode(const QString &code)
{
    if (outputEdit)
        outputEdit->updateCode(code);
}

void TranslationPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    inputEdit = new CodeEditComponent(this);
    inputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    inputEdit->showButtons(CodeEditComponent::None);
    inputEdit->setTitle(tr("Input Code"));
    inputEdit->setFixedHeight(200);
    layout->addWidget(inputEdit);

    layout->addSpacing(20);

    QHBoxLayout *midLayout = new QHBoxLayout;

    langComboBox = new DComboBox(this);
    langComboBox->setFixedSize(192, 36);
    langComboBox->addItems(CodeGeeX::SupportLanguage);
    midLayout->addWidget(langComboBox);

    midLayout->addStretch(1);

    transBtn = new DPushButton(this);
    transBtn->setText(tr("Translate"));
    transBtn->setFixedSize(78, 36);
    midLayout->addWidget(transBtn);

    layout->addLayout(midLayout);

    outputEdit = new CodeEditComponent(this);
    outputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    outputEdit->setTitle(tr("Output Code"));
    outputEdit->showButtons(CodeEditComponent::InsertOnly);
    outputEdit->setReadOnly(true);
    outputEdit->setUpdateHeight(true);
    outputEdit->updateCode("");

    layout->addWidget(outputEdit);

    layout->addStretch(1);
}

void TranslationPageWidget::initConnection()
{
    connect(transBtn, &DPushButton::clicked, this, &TranslationPageWidget::onTranslateBtnClicked);
    connect(Copilot::instance(), &Copilot::translatedResult, this, &TranslationPageWidget::onRecevieTransCode);
}

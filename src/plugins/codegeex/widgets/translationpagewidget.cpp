// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "translationpagewidget.h"
#include "codeeditcomponent.h"
#include "copilot.h"
#include "codegeex/copilotapi.h"

#include <DComboBox>
#include <DPushButton>
#include <DLabel>
#include <DSpinner>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

TranslationPageWidget::TranslationPageWidget(QWidget *parent)
    : DWidget(parent)
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
    spinner->move(outputEdit->x() + outputEdit->width() / 2, outputEdit->y() + outputEdit->height() / 2);
    spinner->show();
    spinner->start();

    QString dstLang = langComboBox->currentText();
    QString srcCode = inputEdit->getContent();
    Copilot::instance()->translateCode(srcCode, dstLang);
}

void TranslationPageWidget::onRecevieTransCode(const QString &code, const QString &dstLang)
{
    spinner->stop();
    spinner->hide();

    if (outputEdit)
        outputEdit->updateCode(code, dstLang);
}

void TranslationPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    inputEdit = new CodeEditComponent(this);
    inputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    inputEdit->showButtons(CodeEditComponent::None);
    inputEdit->setTitle(tr("Input Code"));
    inputEdit->setPlaceholderText(tr("Please input the code to be translated"));
    inputEdit->setFixedHeight(280);
    layout->addWidget(inputEdit);

    QHBoxLayout *midLayout = new QHBoxLayout;

    langComboBox = new DComboBox(this);
    langComboBox->setFixedSize(192, 36);
    langComboBox->addItems(CodeGeeX::SupportLanguage);
    midLayout->addWidget(langComboBox);

    transBtn = new DSuggestButton(this);
    transBtn->setText(tr("Translate"));
    transBtn->setFixedSize(78, 36);
    midLayout->addWidget(transBtn);

    layout->addLayout(midLayout);

    outputEdit = new CodeEditComponent(this);
    outputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    outputEdit->setTitle(tr("Output Code"));
    outputEdit->showButtons(CodeEditComponent::InsertOnly);
    outputEdit->setReadOnly(true);
    outputEdit->setUpdateHeight(false);
    outputEdit->updateCode("");

    spinner = new DSpinner(this);
    spinner->setFixedSize(21, 21);
    spinner->hide();

    outputEdit->setContentsMargins(0, 10, 0, 0);
    layout->addWidget(outputEdit);
}

void TranslationPageWidget::initConnection()
{
    connect(transBtn, &DPushButton::clicked, this, &TranslationPageWidget::onTranslateBtnClicked);
    connect(Copilot::instance(), &Copilot::translatedResult, this, &TranslationPageWidget::onRecevieTransCode);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "translationpagewidget.h"
#include "codeeditcomponent.h"
#include "copilot.h"
#include "codegeex/copilotapi.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

TranslationPageWidget::TranslationPageWidget(QWidget *parent)
    : QWidget (parent)
{
    initUI();
    initConnection();
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

    QHBoxLayout *midLayout = new QHBoxLayout;

    QVBoxLayout *comboBoxLayout = new QVBoxLayout;
    QLabel *comboBoxLabel = new QLabel(this);
    comboBoxLabel->setFixedHeight(20);
    comboBoxLabel->setText(tr("Translate Into:"));
    comboBoxLayout->addWidget(comboBoxLabel);

    layout->addSpacing(20);

    langComboBox = new QComboBox(this);
    langComboBox->addItems(CodeGeeX::SupportLanguage);
    comboBoxLayout->addWidget(langComboBox);

    midLayout->addLayout(comboBoxLayout);

    midLayout->addSpacing(100);

    transBtn = new QPushButton(this);
    transBtn->setText(tr("Translate"));
    midLayout->addWidget(transBtn);

    layout->addLayout(midLayout);

    layout->addSpacing(20);

    outputEdit = new CodeEditComponent(this);
    outputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    outputEdit->setTitle(tr("Output Code"));
    outputEdit->setFixedHeight(200);
    outputEdit->showButtons(CodeEditComponent::InsertOnly);
    outputEdit->setReadOnly(true);
    layout->addWidget(outputEdit);

    layout->addStretch(1);
}

void TranslationPageWidget::initConnection()
{
    connect(transBtn, &QPushButton::clicked, this, &TranslationPageWidget::onTranslateBtnClicked);
    connect(Copilot::instance(), &Copilot::translatedResult, this, &TranslationPageWidget::onRecevieTransCode);
}

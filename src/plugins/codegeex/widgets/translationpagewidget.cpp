// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "translationpagewidget.h"
#include "codeeditcomponent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

TranslationPageWidget::TranslationPageWidget(QWidget *parent)
    : QWidget (parent)
{
    initUI();
}

void TranslationPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    CodeEditComponent *inputEdit = new CodeEditComponent(this);
    inputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    inputEdit->showButtons(CodeEditComponent::None);
    inputEdit->setTitle("Input Code");
    inputEdit->setFixedHeight(200);
    layout->addWidget(inputEdit);

    QHBoxLayout *midLayout = new QHBoxLayout;

    QVBoxLayout *comboBoxLayout = new QVBoxLayout;
    QLabel *comboBoxLabel = new QLabel(this);
    comboBoxLabel->setFixedHeight(20);
    comboBoxLabel->setText(tr("Translate Into:"));
    comboBoxLayout->addWidget(comboBoxLabel);

    layout->addSpacing(20);

    QComboBox *box = new QComboBox(this);
    box->addItems({ "c++", "c", "python", "java", "javascript" });
    comboBoxLayout->addWidget(box);

    midLayout->addLayout(comboBoxLayout);

    midLayout->addSpacing(100);

    QPushButton *transButton = new QPushButton(this);
    transButton->setText(tr("Translate"));
    midLayout->addWidget(transButton);

    layout->addLayout(midLayout);

    layout->addSpacing(20);

    CodeEditComponent *outputEdit = new CodeEditComponent(this);
    outputEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    outputEdit->setTitle(tr("Output Code"));
    outputEdit->setFixedHeight(200);
    outputEdit->showButtons(CodeEditComponent::InsertOnly);
    outputEdit->setReadOnly(true);
    layout->addWidget(outputEdit);

    layout->addStretch(1);
}

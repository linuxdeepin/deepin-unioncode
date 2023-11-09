// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditcomponent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QDebug>

CodeEditComponent::CodeEditComponent(QWidget *parent)
    : QWidget (parent)
{
    initUI();
}

void CodeEditComponent::showButtons(CodeEditComponent::ShowButtonsType type)
{
    titleWidget->setVisible(true);
    switch (type) {
    case InsertOnly:
        insertButton->setVisible(true);
        copyButton->setVisible(false);
        break;
    case CopyOnly:
        insertButton->setVisible(false);
        copyButton->setVisible(true);
        break;
    case CopyAndInsert:
        insertButton->setVisible(true);
        copyButton->setVisible(true);
        break;
    default:
        insertButton->setVisible(false);
        copyButton->setVisible(false);
        break;
    }
}

void CodeEditComponent::setTitle(const QString &title)
{
    titleWidget->setVisible(true);
    this->title->setText(title);
}

void CodeEditComponent::setReadOnly(bool readOnly)
{
    if (codeEdit)
        codeEdit->setReadOnly(readOnly);
}

void CodeEditComponent::setUpdateHeight(bool update)
{
    heightUpdate = update;
}

void CodeEditComponent::updateCode(const QString &code)
{
    if (codeEdit) {
        codeEdit->setPlainText(code);
        if (heightUpdate) {
            QTextDocument *doc = codeEdit->document();
            qreal height = doc->size().height() * codeEdit->fontMetrics().height();
            height += 15;
            codeEdit->setFixedHeight(static_cast<int>(height));
        }
    }
}

void CodeEditComponent::updateCode(const QStringList &codeLines)
{
    QString code {};
    for (auto line : codeLines) {
        code += line;
        code += "\n";
    }
    code.chop(1);
    updateCode(code);
}

void CodeEditComponent::cleanFinalLine()
{
    QString text = codeEdit->toPlainText();
    if (text.endsWith("``"))
        text.chop(3);
    if (text.endsWith("`"))
        text.chop(2);
    updateCode(text);
}

void CodeEditComponent::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    titleWidget = new QWidget(this);
    layout->addWidget(titleWidget);
    initTitleWidgets();
    titleWidget->setVisible(false);

    codeEdit = new QPlainTextEdit(this);
    codeEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    layout->addWidget(codeEdit);
}

void CodeEditComponent::initTitleWidgets()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setDirection(QHBoxLayout::RightToLeft);
    titleWidget->setLayout(layout);

    copyButton = new QPushButton(titleWidget);
    copyButton->setText(tr("Copy"));
    layout->addWidget(copyButton);

    insertButton = new QPushButton(titleWidget);
    insertButton->setText(tr("Insert"));
    layout->addWidget(insertButton);

    layout->addStretch(1);

    title = new QLabel(titleWidget);
    title->setText("");
    layout->addWidget(title);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditcomponent.h"
#include "copilot.h"

#include <DApplication>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

CodeEditComponent::CodeEditComponent(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnection();
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

QString CodeEditComponent::getContent()
{
    return codeEdit->toPlainText();
}

void CodeEditComponent::onInsertBtnClicked()
{
    if (!codeEdit)
        return;

    if (codeEdit->toPlainText().isEmpty())
        return;

    Copilot::instance()->insterText(codeEdit->toPlainText());
}

void CodeEditComponent::onCopyBtnClicked()
{
    if (!codeEdit)
        return;

    if (codeEdit->toPlainText().isEmpty())
        return;

    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setMimeData(new QMimeData);
    clipboard->setText(codeEdit->toPlainText());
}

void CodeEditComponent::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    titleWidget = new DWidget(this);
    layout->addWidget(titleWidget);
    initTitleWidgets();
    titleWidget->setVisible(false);

    codeEdit = new DPlainTextEdit(this);
    codeEdit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    codeEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    layout->addWidget(codeEdit);
}

void CodeEditComponent::initTitleWidgets()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setDirection(QHBoxLayout::RightToLeft);
    titleWidget->setLayout(layout);

    copyButton = new DPushButton(titleWidget);
//    copyButton->setText(tr("Copy"));
    copyButton->setFlat(true);
    copyButton->setIcon(QIcon(QPixmap(":/resoures/images/copy.png")));
    layout->addWidget(copyButton);

    insertButton = new DPushButton(titleWidget);
//    insertButton->setText(tr("Insert"));
    insertButton->setFlat(true);
    insertButton->setIcon(QIcon(QPixmap(":/resoures/images/insert.svg")));
    layout->addWidget(insertButton);

    layout->addStretch(1);

    title = new DLabel(titleWidget);
    title->setText("");
    layout->addWidget(title);
}

void CodeEditComponent::initConnection()
{
    connect(copyButton, &DPushButton::clicked, this, &CodeEditComponent::onCopyBtnClicked);
    connect(insertButton, &DPushButton::clicked, this, &CodeEditComponent::onInsertBtnClicked);
}

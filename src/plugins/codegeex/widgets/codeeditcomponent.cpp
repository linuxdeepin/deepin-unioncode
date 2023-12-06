// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditcomponent.h"
#include "copilot.h"

#include <DApplication>
#include <DHorizontalLine>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QBitmap>

CodeEditComponent::CodeEditComponent(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnection();
}

void CodeEditComponent::showButtons(CodeEditComponent::ShowButtonsType type)
{
    hLine->setVisible(true);
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
        hLine->setVisible(false);
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

void CodeEditComponent::setPlaceholderText(const QString &text)
{
    if (codeEdit)
        codeEdit->setPlaceholderText(text);
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
            qreal height = doc->lineCount() * codeEdit->fontMetrics().height();
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

void CodeEditComponent::paintEvent(QPaintEvent *event)
{
    auto pa = palette();
    if (pa.color(QPalette::Window) != pa.color(QPalette::Base)) {
        pa.setColor(QPalette::Window, pa.color(QPalette::Base));
        setPalette(pa);
    }

    QStyleOption opt;
    opt.initFrom(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QBitmap bmp(size());
    bmp.fill();
    QPainter painter(&bmp);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.drawRoundedRect(bmp.rect(), 8, 8);
    setMask(bmp);

    DWidget::paintEvent(event);
}

void CodeEditComponent::initUI()
{
    setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    titleWidget = new DWidget(this);
    titleWidget->setFixedHeight(36);
    layout->addWidget(titleWidget);
    initTitleWidgets();
    titleWidget->setVisible(false);

    initButton();

    editFrame = new DFrame(this);
    auto editLayout = new QVBoxLayout(editFrame);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editFrame->setLayout(editLayout);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 10, 0);
    buttonLayout->setAlignment(Qt::AlignRight);

    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(insertButton);

    codeEdit = new DTextEdit(this);
    codeEdit->setFrameShape(QFrame::NoFrame);
    codeEdit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    codeEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    hLine = new DHorizontalLine;
    hLine->setVisible(false);

    editLayout->setSpacing(0);
    editLayout->addLayout(buttonLayout);
    editLayout->addWidget(hLine);
    editLayout->addWidget(codeEdit);

    layout->addWidget(editFrame);
}

void CodeEditComponent::initTitleWidgets()
{
    QHBoxLayout *layout = new QHBoxLayout;
    titleWidget->setLayout(layout);

    title = new DLabel(titleWidget);
    title->setText("");
    layout->addWidget(title);
}

void CodeEditComponent::initButton()
{
    copyButton = new DPushButton(this);
    copyButton->setFlat(true);
    copyButton->setIcon(QIcon::fromTheme("codegeex_copy"));
    copyButton->setFixedSize(QSize(24, 24));

    insertButton = new DPushButton(this);
    insertButton->setFlat(true);
    insertButton->setIcon(QIcon::fromTheme("codegeex_insert"));
    insertButton->setFixedSize(QSize(24, 24));
}

void CodeEditComponent::initConnection()
{
    connect(copyButton, &DPushButton::clicked, this, &CodeEditComponent::onCopyBtnClicked);
    connect(insertButton, &DPushButton::clicked, this, &CodeEditComponent::onInsertBtnClicked);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultwindow.h"
#include "searchresulttreeview.h"
#include "resulttemdelegate.h"

#include <DPushButton>
#include <DMessageBox>
#include <DLabel>
#include <DLineEdit>
#include <DIconButton>
#include <DDialog>
#include <DIconTheme>

#include <QVBoxLayout>
#include <QDebug>
#include <QPalette>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
class SearchResultWindowPrivate
{
    SearchResultWindowPrivate() {}
    SearchResultTreeView *treeView { nullptr };
    QWidget *replaceWidget { nullptr };
    DLineEdit *replaceEdit { nullptr };
    DLabel *resultLabel { nullptr };
    QLabel *iconLabel { nullptr };
    DDialog *replaceTextDialog { nullptr };
    DDialog *replaceWarningDialog { nullptr };
    DPushButton *replaceBtn { nullptr };
    bool replaceTextFlag = false;
    SearchParams searchParams;
    int resultCount { 0 };

    friend class SearchResultWindow;
};

SearchResultWindow::SearchResultWindow(QWidget *parent)
    : QWidget(parent), d(new SearchResultWindowPrivate())
{
    setupUi();

    qRegisterMetaType<FindItemList>("FindItemList");
}

SearchResultWindow::~SearchResultWindow()
{
    delete d;
}

void SearchResultWindow::clear()
{
    d->iconLabel->setVisible(true);
    d->treeView->clearData();
}

void SearchResultWindow::setupUi()
{
    d->replaceWidget = new QWidget(this);
    QHBoxLayout *replaceLayout = new QHBoxLayout();

    d->replaceEdit = new DLineEdit(this);
    d->replaceEdit->setFixedWidth(280);
    d->replaceEdit->setPlaceholderText(tr("Replace"));
    d->replaceBtn = new DPushButton(DPushButton::tr("Replace"), this);
    d->replaceBtn->setFixedSize(120, 36);
    d->replaceWidget->setLayout(replaceLayout);

    replaceLayout->addWidget(d->replaceEdit, 0, Qt::AlignLeft);
    replaceLayout->addWidget(d->replaceBtn, 0, Qt::AlignLeft);
    replaceLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DIconButton *cleanBtn = new DIconButton(this);   //Clean && Return
    cleanBtn->setIcon(DIconTheme::findQIcon("go-previous"));
    QSize iconSize(12, 12);
    cleanBtn->setIconSize(iconSize);

    cleanBtn->setFixedSize(36, 36);
    d->resultLabel = new DLabel(this);

    hLayout->addWidget(cleanBtn);
    hLayout->addWidget(d->replaceWidget);
    hLayout->addWidget(d->resultLabel);
    hLayout->setAlignment(Qt::AlignLeft);

    d->treeView = new SearchResultTreeView(this);
    d->treeView->setHeaderHidden(true);
    d->treeView->setLineWidth(0);
    d->treeView->setItemDelegate(new ResultItemDelegate(this));

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->treeView, 1);

    d->iconLabel = new QLabel(this);
    QVBoxLayout *iconLayout = new QVBoxLayout();
    d->iconLabel->setPixmap(DIconTheme::findQIcon("find_noResults").pixmap(QSize(96, 96)));
    iconLayout->addWidget(d->iconLabel, Qt::AlignCenter);

    iconLayout->setAlignment(Qt::AlignCenter);
    vLayout->addLayout(iconLayout);
    vLayout->setAlignment(iconLayout, Qt::AlignCenter);

    connect(cleanBtn, &DIconButton::clicked, this, &SearchResultWindow::clean);
    connect(d->replaceBtn, &DPushButton::clicked, this, &SearchResultWindow::replace);

    setLayout(vLayout);
    setRepalceWidgtVisible(false);
}

void SearchResultWindow::setRepalceWidgtVisible(bool visible)
{
    d->replaceBtn->setEnabled(!visible);
    d->replaceWidget->setVisible(visible);
}

void SearchResultWindow::appendResults(const FindItemList &itemList)
{
    d->treeView->setVisible(true);
    d->iconLabel->setVisible(false);
    d->treeView->appendData(itemList);
    d->resultCount += itemList.count();
    QString msg = tr("%1 matches found.").arg(d->resultCount);
    showMsg(true, msg);
}

void SearchResultWindow::searchFinished()
{
    if (d->resultCount > 0) {
        QString msg = tr("Search completed, %1 matches found.").arg(d->resultCount);
        showMsg(true, msg);
        d->replaceBtn->setEnabled(true);
        return;
    }

    d->treeView->setVisible(false);
    d->iconLabel->setVisible(true);
    showMsg(false, tr("No match found!"));
}

void SearchResultWindow::replaceFinished(bool success)
{
    QString msg = success ? tr("Replacement successful!") : tr("Replace failed!");
    showMsg(success, msg);
}

void SearchResultWindow::clean()
{
    d->resultCount = 0;
    d->treeView->clearData();
    emit reqBack();
}

void SearchResultWindow::replace()
{
    showMsg(true, tr("Replacing, please wait..."));
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        d->replaceTextFlag = false;
        d->replaceTextDialog = new DDialog(this);
        d->replaceTextDialog->setIcon(DIconTheme::findQIcon("dialog-warning"));
        d->replaceTextDialog->setMessage(tr("Repalce text is empty, will continue?"));
        d->replaceTextDialog->insertButton(0, tr("No"));
        d->replaceTextDialog->insertButton(1, tr("Yes"), true, DDialog::ButtonRecommend);

        connect(d->replaceTextDialog, &DDialog::buttonClicked, this, [=](int index) {
            if (index == 0) {
                d->replaceTextFlag = true;
                d->replaceTextDialog->reject();
            } else if (index == 1) {
                d->replaceTextDialog->accept();
            }
        });
        d->replaceTextDialog->exec();
    }

    if (d->replaceTextFlag)
        return;

    Q_EMIT reqReplace(replaceText);
}

void SearchResultWindow::showMsg(bool succeed, QString msg)
{
    QPalette palette = d->resultLabel->palette();
    QColor textColor = this->palette().color(QPalette::WindowText);

    int red, green, blue, alpha;
    textColor.getRgb(&red, &green, &blue, &alpha);
    // 降低透明度
    alpha = static_cast<int>(alpha * 0.5);   // 0.5 表示减少50%

    QColor newColor = QColor::fromRgb(red, green, blue, alpha);

    palette.setColor(QPalette::WindowText, newColor);
    d->resultLabel->setPalette(palette);
    d->resultLabel->setText(msg);
}

QStringList SearchResultWindow::resultFileList() const
{
    return d->treeView->resultFileList();
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findtoolbar.h"
#include "common/common.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DIconButton>

#include <QVBoxLayout>
#include <QHBoxLayout>

class FindToolBarPrivate
{
    FindToolBarPrivate(){}
    DLineEdit *findLineEdit{nullptr};
    DLineEdit *replaceLineEdit{nullptr};

    friend class FindToolBar;
};

FindToolBar::FindToolBar(QWidget *parent)
    : QWidget(parent)
    , d(new FindToolBarPrivate())
{
    setupUi();
}

void FindToolBar::setupUi()
{
    setMaximumHeight(85);
    setMinimumWidth(800);

    DWidget *mainFrame = new DWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();
    mainFrame->setLayout(hLayout);
    d->findLineEdit = new DLineEdit();        //搜索框
    d->findLineEdit->setPlaceholderText(tr("Find"));
    d->replaceLineEdit = new DLineEdit();     //替换框
    d->replaceLineEdit->setPlaceholderText(tr("Replace"));

    DIconButton *findPreBtn = new DIconButton(this);
    DIconButton *findNextBtn = new DIconButton(this);
    findPreBtn->setIcon(QIcon::fromTheme("go-up"));
    findNextBtn->setIcon(QIcon::fromTheme("go-down"));
    DPushButton *replaceBtn = new DPushButton(QPushButton::tr("Replace"));     //替换按钮
    DPushButton *replaceAllBtn = new DPushButton(QPushButton::tr("Repalce All"));   //全部替换按钮
    DPushButton *replaceFindBtn = new DPushButton(QPushButton::tr("Replace && Find"));      //替换别查找

    hLayout->addWidget(d->findLineEdit);
    hLayout->addWidget(d->replaceLineEdit);
    hLayout->addWidget(findPreBtn);
    hLayout->addWidget(findNextBtn);
    hLayout->addWidget(replaceBtn);
    hLayout->addWidget(replaceAllBtn);
    hLayout->addWidget(replaceFindBtn);

    vLayout->addWidget(mainFrame);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);

    setLayout(vLayout);

    connect(findPreBtn, &QAbstractButton::clicked, this, &FindToolBar::findPrevious);
    connect(findNextBtn, &QAbstractButton::clicked, this, &FindToolBar::findNext);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolBar::replace);
    connect(replaceFindBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceSearch);
    connect(replaceAllBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceAll);
}

void FindToolBar::handleFindActionTriggered() {
    d->findLineEdit->setFocus();
}

void FindToolBar::findPrevious()
{
    QString text = d->findLineEdit->text();
    if (text.isEmpty())
        return;
    editor.searchText(text, FindType::Previous);
}

void FindToolBar::findNext()
{
    QString text = d->findLineEdit->text();
    if (text.isEmpty())
        return;
    editor.searchText(text, FindType::Next);
}

void FindToolBar::replace()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    editor.replaceText(srcText, destText, RepalceType::Repalce);
}

void FindToolBar::replaceSearch()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    editor.replaceText(srcText, destText, RepalceType::FindAndReplace);
}

void FindToolBar::replaceAll()
{
    QString srcText = d->findLineEdit->text();
    if (srcText.isEmpty())
        return;
    QString destText = d->replaceLineEdit->text();
    editor.replaceText(srcText, destText, RepalceType::RepalceAll);
}


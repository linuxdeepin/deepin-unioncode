// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commentconfigwidget.h"
#include "commentconfigdetailwidget.h"
#include "settingsdefine.h"
#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DFrame>
#include <DPushButton>
#include <DTabWidget>
#include <DMessageBox>

#include <QHBoxLayout>

#include <iostream>

DWIDGET_USE_NAMESPACE

class CommentConfigWidgetPrivate
{
    DFrame *btnFrame = nullptr;
    
    DTabWidget *tabWidget = nullptr;
    DPushButton *addGroupBtn = nullptr;
    DPushButton *delGroupBtn = nullptr;
    
    friend class CommentConfigWidget;
};

CommentConfigWidget::CommentConfigWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new CommentConfigWidgetPrivate())
{
    initUI();
    initConnections();
}

CommentConfigWidget::~CommentConfigWidget()
{
    delete d;
}

void CommentConfigWidget::initUI()
{
    d->tabWidget = new DTabWidget(this);
    QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                         EditorConfig, Node::MimeTypeConfig, map);
    if (map.count() == 0) {
        d->tabWidget->addTab(new CommentConfigDetailWidget(this, 0), tr("Defualt"));
    }

    int currentTabWidgetCount = 0;
    while (currentTabWidgetCount < map.count()) {
        QMap<QString, QVariant> mimeTypeMap = map.value(QString::number(currentTabWidgetCount)).toMap();
        d->tabWidget->addTab(new CommentConfigDetailWidget(this, currentTabWidgetCount), 
            mimeTypeMap.value(Key::MimeTypeGroupName).toString());
        currentTabWidgetCount++;
    }
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    d->addGroupBtn = new DPushButton(tr("Add Group"), this);
    d->delGroupBtn = new DPushButton(tr("Delete Group"), this);
    
    btnLayout->addWidget(d->addGroupBtn);
    btnLayout->addWidget(d->delGroupBtn);
    btnLayout->setSpacing(50);
    btnLayout->setContentsMargins(50, 0, 50, 0);
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    
    mainLayout->addWidget(d->tabWidget);
    mainLayout->addLayout(btnLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    this->setLayout(mainLayout);
}

void CommentConfigWidget::initConnections()
{
    connect(d->addGroupBtn, &DPushButton::clicked, this, [ = ]() {
        CommentConfigDetailWidget *detailWidget = new CommentConfigDetailWidget(this, d->tabWidget->count());
        d->tabWidget->addTab(detailWidget, tr("New Group"));
    });
    
    connect(d->delGroupBtn, &DPushButton::clicked, this, [ = ]() {
        if (d->tabWidget->count() > 1) {
            d->tabWidget->removeTab(d->tabWidget->currentIndex());
        } else {
            DMessageBox::warning(this, tr("Warning"), tr("You can't delete the last group!"));
        }
    });
}

void CommentConfigWidget::setUserConfig(const QMap<QString, QVariant> &map)
{   
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *detailWidget = dynamic_cast<CommentConfigDetailWidget *>(d->tabWidget->widget(index));
        if (detailWidget) {
            detailWidget->setUserConfig(map);
        }
    }
}

void CommentConfigWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *detailWidget = dynamic_cast<CommentConfigDetailWidget *>(d->tabWidget->widget(index));
        if (detailWidget) {
            detailWidget->getUserConfig(map);
        }
    }
}

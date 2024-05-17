// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commentconfigwidget.h"
#include "commentconfigdetailwidget.h"
#include "settingsdefine.h"
#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"
#include "editorsettings.h"

#include <DFrame>
#include <DPushButton>
#include <DTabWidget>
#include <DDialog>

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
    if (map.isEmpty() || map.value("0").toMap().value(Key::MimeTypeGroupName).toString().isEmpty()) {
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "0", Key::MimeType, "text/x-c++src;text/x-chdr;text/x-c++hdr;text/x-java;application/javascript");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "0", Key::SingleLineComment, "//");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "0", Key::StartMultiLineComment, "/*");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "0", Key::EndMultiLineComment, "*/");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "0", Key::MimeTypeGroupName, "C++/Java/Javascript");
        
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "1", Key::MimeType, "text/x-python;text/x-cmake;application/x-shellscript");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "1", Key::SingleLineComment, "#");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "1", Key::StartMultiLineComment, "\'\'\'");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "1", Key::EndMultiLineComment, "\'\'\'");
        EditorSettings::instance()->setValue(Node::MimeTypeConfig, "1", Key::MimeTypeGroupName, "Python/CMake/Shell");
    }

    int currentTabWidgetCount = 0;
    while (currentTabWidgetCount < EditorSettings::instance()->getMap(Node::MimeTypeConfig).count()) {
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
            DDialog dialog;
            dialog.setMessage(tr("Are you sure to delete this group?"));
            dialog.setWindowTitle(tr("Delete Warining"));
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.insertButton(0, tr("Ok"));
            dialog.insertButton(1, tr("Cancel"));
            if (dialog.exec() == 0) 
                d->tabWidget->removeTab(d->tabWidget->currentIndex());
        } else {
            DDialog dialog;
            dialog.setMessage(tr("You can't delete the last group!"));
            dialog.setWindowTitle(tr("Delete Warining"));
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.insertButton(0, tr("Ok"));
            dialog.exec();
        }
    });
}

void CommentConfigWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        d->tabWidget->setTabText(index, EditorSettings::instance()->getMap(Node::MimeTypeConfig)
            .value(QString::number(index)).toMap().value(Key::MimeTypeGroupName).toString());
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

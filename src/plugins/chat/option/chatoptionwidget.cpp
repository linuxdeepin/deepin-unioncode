// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatoptionwidget.h"
#include "detailwidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

static const char *kCATEGORY_CHAT = "Chat";

class ChatOptionWidgetPrivate
{
    DTabWidget *tabWidget = nullptr;
    friend class ChatOptionWidget;
};

ChatOptionWidget::ChatOptionWidget(QWidget *parent)
    : PageWidget(parent), d(new ChatOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    d->tabWidget->setDocumentMode(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new DetailWidget(), "Detail");
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

ChatOptionWidget::~ChatOptionWidget()
{
    if (d)
        delete d;
}

void ChatOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(kCATEGORY_CHAT, itemNode, map);
        }
    }
}

void ChatOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(kCATEGORY_CHAT, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}

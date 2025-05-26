// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custommodelsoptionwidget.h"
#include "detailwidget.h"
#include "aimanager.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class CustomModelsOptionWidgetPrivate
{
    DTabWidget *tabWidget = nullptr;
    friend class CustomModelsOptionWidget;
};

CustomModelsOptionWidget::CustomModelsOptionWidget(QWidget *parent)
    : PageWidget(parent), d(new CustomModelsOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    d->tabWidget = new DTabWidget(this);
    d->tabWidget->tabBar()->setAutoHide(true);
    d->tabWidget->setDocumentMode(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new DetailWidget(d->tabWidget), kCATEGORY_OPTIONKEY);
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, this, [this]() {
        readConfig();
    });
}

CustomModelsOptionWidget::~CustomModelsOptionWidget()
{
    if (d)
        delete d;
}

void CustomModelsOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(kCATEGORY_CUSTOMMODELS, itemNode, map);
        }
    }
    AiManager::instance()->readLLMFromOption();
}

void CustomModelsOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(kCATEGORY_CUSTOMMODELS, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}

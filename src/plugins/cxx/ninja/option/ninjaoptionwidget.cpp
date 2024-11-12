// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjaoptionwidget.h"
#include "ninjawidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <DTabWidget>

DWIDGET_USE_NAMESPACE

class NinjaOptionWidgetPrivate
{
    DTabWidget *tabWidget = nullptr;

    friend class NinjaOptionWidget;
};

NinjaOptionWidget::NinjaOptionWidget(QWidget *parent)
    : PageWidget(parent), d(new NinjaOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new NinjaWidget(), tr("Ninja"));
    d->tabWidget->setDocumentMode(true);
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

NinjaOptionWidget::~NinjaOptionWidget()
{
    if (d)
        delete d;
}

void NinjaOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(option::CATEGORY_NINJA, itemNode, map);
        }
    }
}

void NinjaOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_NINJA, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}

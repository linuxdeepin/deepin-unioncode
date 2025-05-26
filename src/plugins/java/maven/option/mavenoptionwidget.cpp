// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenoptionwidget.h"
#include "mavenwidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
class MavenOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class MavenOptionWidget;
};

MavenOptionWidget::MavenOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new MavenOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);
    //去除左/上边的边框
    d->tabWidget->setDocumentMode(true);

    d->tabWidget->addTab(new MavenWidget(), tr("Maven"));

    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

MavenOptionWidget::~MavenOptionWidget()
{
    if (d)
        delete d;
}

void MavenOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(option::CATEGORY_MAVEN, itemNode, map);
        }
    }
}

void MavenOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_MAVEN, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}





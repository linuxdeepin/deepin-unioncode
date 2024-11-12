// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "javaoptionwidget.h"
#include "jdkwidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>
DWIDGET_USE_NAMESPACE
class JavaOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class JavaOptionWidget;
};

JavaOptionWidget::JavaOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new JavaOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    layout->addWidget(d->tabWidget);
    d->tabWidget->tabBar()->setAutoHide(true);
    //去除左/上边的边框
    d->tabWidget->setDocumentMode(true);

    d->tabWidget->addTab(new JDKWidget(), tr("JDK"));
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

JavaOptionWidget::~JavaOptionWidget()
{
    if (d)
        delete d;
}

void JavaOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(option::CATEGORY_JAVA, itemNode, map);
        }
    }
}

void JavaOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_JAVA, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}


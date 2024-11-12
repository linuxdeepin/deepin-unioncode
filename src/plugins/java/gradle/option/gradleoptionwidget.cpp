// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradleoptionwidget.h"
#include "gradlewidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>
#include <QStylePainter>
DWIDGET_USE_NAMESPACE

class GradleOptionWidgetPrivate
{
    DTabWidget *tabWidget = nullptr;

    friend class GradleOptionWidget;
};

GradleOptionWidget::GradleOptionWidget(QWidget *parent)
    : PageWidget(parent), d(new GradleOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new GradleWidget(), tr("Gradle"));
    //去除左/上边的边框
    d->tabWidget->setDocumentMode(true);

    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

GradleOptionWidget::~GradleOptionWidget()
{
    if (d)
        delete d;
}

void GradleOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(option::CATEGORY_GRADLE, itemNode, map);
        }
    }
}

void GradleOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_GRADLE, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}

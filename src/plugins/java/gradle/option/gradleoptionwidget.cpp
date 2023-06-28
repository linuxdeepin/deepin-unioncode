// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradleoptionwidget.h"
#include "gradlewidget.h"

#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <QTabWidget>

class GradleOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class GradleOptionWidget;
};

GradleOptionWidget::GradleOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new GradleOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new QTabWidget();
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new GradleWidget(), tr("Gradle"));
    QObject::connect(d->tabWidget, &QTabWidget::currentChanged, [this]() {
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
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(),
                                          option::CATEGORY_GRADLE, itemNode, map);

            OptionManager::getInstance()->updateData();
        }
    }
}

void GradleOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                         option::CATEGORY_GRADLE, itemNode, map);
            pageWidget->setUserConfig(map);
        }
    }
}


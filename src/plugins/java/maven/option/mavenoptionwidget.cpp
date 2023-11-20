// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenoptionwidget.h"
#include "mavenwidget.h"

#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <QTabWidget>

class MavenOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class MavenOptionWidget;
};

MavenOptionWidget::MavenOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new MavenOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new QTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new MavenWidget(), tr("Maven"));

    QObject::connect(d->tabWidget, &QTabWidget::currentChanged, [this]() {
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
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(),
                                          option::CATEGORY_MAVEN, itemNode, map);

            OptionManager::getInstance()->updateData();
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
            QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                         option::CATEGORY_MAVEN, itemNode, map);
            pageWidget->setUserConfig(map);
        }
    }
}




